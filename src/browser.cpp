/*****************************************************************************
 *                                                                           *
 *  BioSignalML Browser in C++                                               *
 *                                                                           *
 *  Copyright (c) 2014-2015  David Brooks                                    *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *****************************************************************************/

#include "browser.h"
#include "mainwindow.h"
#include "signallist.h"
#include "annotationlist.h"
#include "scroller.h"

#include <QMetaType>
#include <QMessageLogger>

#include <cmath>
#include <exception>

using namespace browser ;


Browser::Browser(bsml::Recording::Ptr recording, float start, float end,
/*====================================================================*/
                       StringDictionary semantic_tags)  //, annotator=None) LAMBDA
: QMainWindow(),
  m_ui(new Ui::MainWindow()),
  m_recording(recording),
  m_modified(false),
  m_closekey(new QShortcut(QKeySequence::Close, this)),
  m_readers(QList<SignalReadThread *>())
{
  float duration ;
  if (isnan(end)) {
    duration = (float)recording->duration() ;
    if (isnan(duration) || duration <= 0.0) {
      duration = 60.0 ;               //#####
      recording->set_duration(duration) ;
      }
    }
  else if (start <= end) {
    duration = end - start ;
    }
  else {
    duration = start - end ;
    start = end ;
    }
  m_start = start ;        //# Used in adjust_layout

  m_signals = new SignalList(this, recording) ; // TODO , annotator) ;
  m_annotations = new AnnotationList(this, recording, semantic_tags) ;
  m_scroller = new Scroller(this, recording, start, duration) ;

  m_ui->setupUi(this, m_signals, m_annotations, m_scroller) ;
  setWindowTitle(((std::string)recording->uri()).c_str()) ;

  // So can be passed between threads using signal/slot
  qRegisterMetaType<bsml::data::TimeSeries::Ptr>("const bsml::data::TimeSeries::Ptr &") ;

  // Close with the close-key shortcut.
  QObject::connect(m_closekey, &QShortcut::activated, this, &Browser::close) ;

  // Setup chart
  m_ui->chartform->setTimeRange(start, duration) ;
  ChartPlot *chart = m_ui->chartform->ui().chart ;
  chart->setId(((std::string)m_recording->uri()).c_str()) ;
  chart->setSemanticTags(semantic_tags) ;
  QObject::connect(chart, &ChartPlot::exportRecording, this, &Browser::exportRecording) ;

  // Connections with signal list
  QObject::connect(m_signals,          &SignalList::add_event_trace,  chart, &ChartPlot::addEventTrace) ;
  QObject::connect(m_signals,          &SignalList::add_signal_trace, chart, &ChartPlot::addSignalTrace) ;
  QObject::connect(m_signals,          &SignalList::show_signals,     this,  &Browser::plot_signals) ;

  QObject::connect(m_signals->model(), &SignalModel::rowVisible,     chart, &ChartPlot::setTraceVisible) ;
  QObject::connect(m_signals->model(), &SignalModel::rowMoved,       chart, &ChartPlot::moveTrace) ;
  QObject::connect(m_signals->ui().signallist, &SignalView::rowSelected, chart, &ChartPlot::plotSelected) ;

  // Connections with annotation list
  QObject::connect(m_annotations, &AnnotationList::annotationAdded,    chart,        &ChartPlot::addAnnotation) ;
  QObject::connect(m_annotations, &AnnotationList::annotationDeleted, chart,         &ChartPlot::deleteAnnotation) ;
  QObject::connect(m_annotations, &AnnotationList::set_marker,        chart,         &ChartPlot::setMarker) ;
  QObject::connect(m_annotations, &AnnotationList::move_plot,         m_scroller,    &Scroller::move_plot) ;
  QObject::connect(m_annotations, &AnnotationList::set_slider_value,  m_scroller,    &Scroller::set_slidervalue) ;
  QObject::connect(m_annotations, &AnnotationList::show_slider_time,  m_scroller,    &Scroller::show_slidertime) ;
  QObject::connect(m_annotations, &AnnotationList::recording_changed, this,          &Browser::set_modified) ;
  QObject::connect(chart,         &ChartPlot::annotationAdded,        m_annotations, &AnnotationList::add_annotation) ;
  QObject::connect(chart,         &ChartPlot::annotationModified,     m_annotations, &AnnotationList::modify_annotation) ;
  QObject::connect(chart,         &ChartPlot::annotationDeleted,      m_annotations, &AnnotationList::delete_annotation) ;

  // Connections with scroller
  QObject::connect(m_scroller, &Scroller::set_plot_timerange, chart, &ChartPlot::setTimeRange) ;
  QObject::connect(m_scroller, &Scroller::show_signals,       this,  &Browser::plot_signals) ;
  QObject::connect(m_scroller, &Scroller::show_annotations,   m_annotations, &AnnotationList::show_annotations) ;

  // Connect our signals
  QObject::connect(this,       &Browser::reset_annotations, chart, &ChartPlot::resetAnnotations) ;
  //#    resize_annotation_list.connect(m_annotations->annotations.resizeCells)
  //#    show_slider_time.connect(m_scroller->show_slider_time)

  // Everything connected, let's go...
  m_signals->plot_signals(start, duration) ;
  m_annotations->show_annotations() ;
  m_scroller->setup_slider() ;

  //    m_ui->chartform._user_zoom_index = m_ui->timezoom.count()
  //    m_ui->chartform.ui.chart.zoomChart.connect(zoom_chart)

  // setFocusPolicy(Qt::Qt.StrongFocus) # Needed to handle key events
  }


Browser::~Browser()
/*---------------*/
{
  stop_readers() ;
  delete m_ui ;
  }

void Browser::stop_readers(void)
/*----------------------------*/
{
  for (auto const &t : m_readers)
    t->stop() ;
  while (1) {
    bool stopped = true ;
    for (auto const &t : m_readers)
      stopped = stopped && t->wait(10) ;
    if (stopped) break ;
    }
  m_readers.clear() ;
  }

void Browser::plot_signals(bsml::Interval::Ptr interval)
/*----------------------------------------------------*/
{
  stop_readers() ;
  emit reset_annotations() ;
  for (auto const &u : m_recording->get_signal_uris()) {   //# Why not only ones currently selected ????  std::cout << "STORED: " << hdf5.serialise_metadata(rdf::Graph::Format::TURTLE) << std::endl ;
    auto s = m_recording->get_signal(u) ;
// What is dynamic type of each signal?? Needs to be HDF5::Signal...
    auto reader = new SignalReadThread(s, interval, m_ui->chartform->ui().chart) ;
    m_readers.append(reader) ;
    reader->start() ;
    }
  }


void Browser::exportRecording(const QString &filename, float start, float end)
/*--------------------------------------------------------------------------*/
//# This is where we create a BSML file with current set of displayed signals
//# along with events and annotations starting or ending in the interval, and
//# provenance linking back to the original.
{
  // TODOprint('export', start, end) ;
  }

//def keyPressEvent(self, event):   ## Also need to do so in chart...
/*-----------------------------    ## And send us hide/show messages or ke*/
//  pass


void Browser::set_modified(const rdf::URI &uri)
/*-------------------------------------------*/
{
  if (m_recording->uri() == uri) m_modified = true ;
  }

void Browser::closeEvent(QCloseEvent *event)
/*----------------------------------------*/
{
  if (m_modified) {
    m_modified = false ;
// Ask user if they want to save...
// Have ^S for Save changes
// Add a menu...
// Multiple main windows, one per file...
    m_recording->close() ;
    }
  QMainWindow::closeEvent(event) ;
  }

