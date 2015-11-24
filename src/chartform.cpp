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

#include "chartform.h"

#include <QKeySequence>

using namespace browser ;


ChartForm::ChartForm(QWidget *parent)
/*=================================*/
: QWidget(parent), // , QtCore.Qt.CustomizeWindowHint
                   // | QtCore.Qt.WindowMinMaxButtonsHint
                   // | QtCore.Qt.WindowStaysOnTopHint)
  m_ui(Ui::Chart())
{
  m_ui.setupUi(this) ;
  QObject::connect(m_ui.chart, &ChartPlot::chartPosition,    this, &ChartForm::chart_resize) ;
  QObject::connect(m_ui.chart, &ChartPlot::updateTimeScroll, this, &ChartForm::position_timescroll) ;
  m_ui.timescroll->hide() ;
  }

void ChartForm::setTimeRange(float start, float duration)
/*-----------------------------------------------------*/
{
  m_ui.chart->setTimeRange(start, duration) ;
  m_ui.chart->setTimeScroll(*m_ui.timescroll) ;
  }

/**
  def setSemanticTags(self, tags):
  #-------------------------------
    m_ui.chart.setSemanticTags(tags)

  def setMarker(self, time):
  #-------------------------
    m_ui.chart.setMarker(time)

  def addSignalPlot(self, id, label, units, visible=True, data=None, ymin=None, ymax=None):
  #----------------------------------------------------------------------------------------
    m_ui.chart.addSignalPlot(id, label, units, visible=visible, data=data, ymin=ymin, ymax=ymax)

  def addEventPlot(self, id, label, mapping=lambda x: str(x), visible=True, data=None):
  #------------------------------------------------------------------------------------
    m_ui.chart.addEventPlot(id, label, mapping, visible=visible, data=data)

  def addAnnotation(self, id, start, end, text, tags, edit=False):
  #---------------------------------------------------------------
    m_ui.chart.addAnnotation(id, start, end, text, tags, edit)

  def deleteAnnotation(self, id):
  #------------------------------
    m_ui.chart.deleteAnnotation(id)

  @pyqtSlot(int, bool)
  def setPlotVisible(self, id, visible=True):
  #------------------------------------------
    m_ui.chart.setPlotVisible(id, visible)

  @pyqtSlot(int, int)
  def movePlot(self, from_id, to_id):
  #----------------------------------
    m_ui.chart.movePlot(from_id, to_id)

  @pyqtSlot(int)
  def plotSelected(self, row):
  #---------------------------
    m_ui.chart.plotSelected(row)

  def orderPlots(self, ids):
  #-------------------------
    m_ui.chart.orderPlots(ids)

  def resetAnnotations(self):
  #--------------------------
    m_ui.chart.resetAnnotations()

  def save_chart_as_png(self, filename):
  #-------------------------------------
    m_ui.chart.save_as_png(filename)
*/

//  def resizeEvent(self, e):
//  #------------------------
//    setGeometry(QtCore.QRect(10, 25, width()-20, height() - 50))

void ChartForm::on_timescroll_valueChanged(int position)
/*----------------------------------------------------*/
{
  m_ui.chart->moveTimeScroll(*m_ui.timescroll) ;
  }

void ChartForm::position_timescroll(bool visible)
/*---------------------------------------------*/
{
  m_ui.chart->setTimeScroll(*m_ui.timescroll) ;
  m_ui.timescroll->setVisible(visible) ;
  }

/**
  def on_timezoom_currentIndexChanged(self, text):
  #-----------------------------------------------
    if isinstance(text, basestring) and text != "":
      scale = float(str(text).split()[0])
      m_ui.chart.setTimeZoom(scale)
      position_timescroll(scale > 1.0)

  @pyqtSlot(float)
  def zoom_chart(self, scale):
  #---------------------------
    if m_ui.timezoom.count() > m_user_zoom_index:
      m_ui.timezoom.setItemText(m_user_zoom_index, "%.2f x" % scale)
    else:
      m_ui.timezoom.insertItem(m_user_zoom_index, "%.2f x" % scale)
    m_ui.timezoom.setCurrentIndex(-1)
    m_ui.timezoom.setCurrentIndex(m_user_zoom_index)
    # Above will trigger on_timezoom_currentIndexChanged()
*/

void ChartForm::on_frame_frameResize(QRect &geometry)
/*-------------------------------------------------*/
{
  geometry.adjust(3, 3, -3, -3) ;
  m_ui.chart->setGeometry(geometry) ;
  //QRect(offset-4, height()-40, width+8, 30))
  }

void ChartForm::chart_resize(int offset, int width, int bottom)
/*-----------------------------------------------------------*/
{
  int h = m_ui.timescroll->height() ;
  m_ui.timescroll->setGeometry(QRect(offset-10, bottom+27, width+40, h)) ;
  }

void ChartForm::on_chart_customContextMenuRequested(const QPoint &pos)
/*------------------------------------------------------------------*/
{
  auto event = QContextMenuEvent(QContextMenuEvent::Mouse, pos) ;
  m_ui.chart->contextMenuEvent(&event) ;
  }
