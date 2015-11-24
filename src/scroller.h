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

#ifndef BROWSER_SCROLLER_H
#define BROWSER_SCROLLER_H

#include "ui_scroller.h"
#include "nrange.h"

#include <biosignalml/biosignalml.h>

#include <QWidget>


namespace browser {

  class Scroller : public QWidget
  /*==============================*/
  {
   Q_OBJECT

   public:
    Scroller(QWidget *parent, bsml::Recording::Ptr recording, float start, float duration) ;

    void setup_slider(void) ;
    void timerEvent(QTimerEvent *event) ;
    void on_segment_valueChanged(int position) ;
    void on_segment_sliderReleased(void) ;

   public slots:
    void show_slidertime(float time) ;
    void set_slidervalue(float time) ;
    void move_plot(float start) ;

   signals:
    void set_plot_timerange(float, float) ;
    void show_signals(bsml::Interval::Ptr) ;
    void show_annotations(void) ;

   private:
    void set_slidertime(QLabel *label, float time) ;
    void stop_movetimer(void) ;
    void start_movetimer(void) ;
    void slider_moved(void) ;

    Ui::Scroller m_ui ;

    bsml::Recording::Ptr m_recording ;
    float m_start ;
    float m_duration ;
    NumericRange m_timerange ;
    int m_movetimer ;
    float m_newstart ;
    bool m_sliding ;
    } ;

  } ;

#endif
