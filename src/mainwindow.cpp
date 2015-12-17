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

#include "mainwindow.h"

#include <QMetaType>
#include <QMessageLogger>

#include <cmath>
#include <exception>

using namespace browser ;


void Ui::MainWindow::setupUi(QMainWindow *mainWindow, QWidget *signalsWidget,
/*-------------------------------------------------------------------------*/
                             QWidget *annotationsWidget, QWidget *scrollWidget)
{
  mainWindow->setObjectName("MainWindow") ;
  mainWindow->resize(1275, 800) ;
  mainWindow->setDockNestingEnabled(false) ;
  mainWindow->setWindowTitle("MainWindow") ;

  centralwidget = new QWidget(mainWindow) ;
  centralwidget->setObjectName("centralwidget") ;

  verticalLayout = new QVBoxLayout(centralwidget) ;
  verticalLayout->setObjectName("verticalLayout") ;
  chartform = new browser::ChartForm(centralwidget) ;
  chartform->setObjectName("chartform") ;
  verticalLayout->addWidget(chartform) ;
  mainWindow->setCentralWidget(centralwidget) ;

  signalsDock = new QDockWidget(mainWindow) ;
  signalsDock->setFloating(false) ;
  signalsDock->setAllowedAreas(Qt::AllDockWidgetAreas) ;
  signalsDock->setObjectName("signalsDock") ;
  signalsWidget->setObjectName("signalsWidget") ;
  signalsDock->setWidget(signalsWidget) ;
  mainWindow->addDockWidget(Qt::LeftDockWidgetArea, signalsDock) ;

  annotationsDock = new QDockWidget(mainWindow) ;
  annotationsDock->setFloating(false) ;
  signalsDock->setAllowedAreas(Qt::AllDockWidgetAreas) ;
  annotationsDock->setObjectName("annotationsDock") ;
  annotationsWidget->setObjectName("annotationsWidget") ;
  annotationsDock->setWidget(annotationsWidget) ;
  mainWindow->addDockWidget(Qt::TopDockWidgetArea, annotationsDock) ;

  scrollDock = new QDockWidget(mainWindow) ;
  scrollDock->setFloating(false) ;
  scrollDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea) ;
  scrollDock->setObjectName("scrollDock") ;
  scrollWidget->setObjectName("scrollWidget") ;
  scrollDock->setWidget(scrollWidget) ;
  mainWindow->addDockWidget(Qt::BottomDockWidgetArea, scrollDock) ;

  QMetaObject::connectSlotsByName(mainWindow) ;
  }


SignalReadThread::SignalReadThread(bsml::Signal::Ptr signal, bsml::Interval::Ptr interval,
/*======================================================================================*/
                                   ChartPlot *plotter)
: QObject(),
  m_signal(signal),
  m_interval(interval),
  m_id(signal_uri(signal)),
  m_exit(true)
{
  QObject::connect(this, &SignalReadThread::append_points, plotter, &ChartPlot::appendData) ;
  moveToThread(&m_thread) ;
  }

void SignalReadThread::start(void)
/*------------------------------*/
{
  m_thread.start() ;
  m_exit = false ;
  emit append_points(m_id, nullptr) ;
  try {
    while (!m_exit) {
      auto d = m_signal->read(m_interval, 20000) ;
      if (d->size() == 0) break ;
      emit append_points(m_id, d) ;
      break ;       // Read needs to be sequential, not absolute....
      }
    }
  catch (std::exception &e) {
    // throw ;  //#######################################
    qCritical("Read thread: %s", e.what()) ;
    m_thread.exit(1) ;
    }
  m_thread.exit(0) ;
  }

void SignalReadThread::stop(void)
/*-----------------------------*/
{
  m_exit = true ;
  }

bool SignalReadThread::wait(unsigned long time)
/*-------------------------------------------*/
{
  return m_thread.wait(time) ;
  }
