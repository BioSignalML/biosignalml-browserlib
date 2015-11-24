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

#include "scroller.h"

using namespace browser ;


Scroller::Scroller(QWidget *parent, bsml::Recording::Ptr recording, float start, float duration)
/*============================================================================================*/
: QWidget(parent),
  m_ui(Ui::Scroller()),
  m_recording(recording),
  m_start(start),
  m_duration(duration),       //## v's recording's duration ???
  m_timerange(NumericRange(0.0, duration))
{
  m_ui.setupUi(this) ;
//  m_ui.rec_posn = new QLabel(this) ;
//  m_ui.rec_posn->setAlignment(Qt::AlignLeft | Qt::AlignTop) ;
  m_ui.rec_posn->resize(m_ui.rec_start->size()) ;
  }

void Scroller::set_slidertime(QLabel *label, float time)
/*----------------------------------------------------*/
//# Show as HH:MM:SS
{
  label->setText(QString("%1").arg(m_timerange.map(time, -1))) ;
  }


void Scroller::show_slidertime(float time)
/*--------------------------------------*/
{
  set_slidertime(m_ui.rec_posn, time) ;
  QScrollBar *sb = m_ui.segment ;
  m_ui.rec_posn->move(  // 50 = approx width of scroll end arrows
    20 + sb->pos().x() + (sb->width()-50)*time/m_duration,
    m_ui.rec_start->pos().y() + 6
    ) ;
  }

void Scroller::set_slidervalue(float time)
/*--------------------------------------*/
{
  QScrollBar *sb = m_ui.segment ;
  int width = sb->maximum() + sb->pageStep() - sb->minimum() ;
  sb->setValue(width*time/m_duration) ;
  }

void Scroller::setup_slider(void)
/*-----------------------------*/
{
  m_sliding = true ;            // So we don't move_plot() when sliderMoved()
  m_movetimer = 0 ;             // is triggered by setting the slider's value
  float duration = (float)m_recording->duration() ;  // Versus slider's duration
  if (duration == 0.0) return ;

  QScrollBar *sb = m_ui.segment ;
  sb->setMinimum(0) ;
  int scrollwidth = 10000 ;
  sb->setPageStep(scrollwidth*m_duration/duration) ;
  sb->setMaximum(scrollwidth - sb->pageStep()) ;
  sb->setValue(scrollwidth*m_start/duration) ;
  set_slidertime(m_ui.rec_start, 0.0) ;
  set_slidertime(m_ui.rec_end, duration) ;
  }

void Scroller::stop_movetimer(void)
/*-------------------------------*/
{
  if (m_movetimer) {
    killTimer(m_movetimer) ;
    m_movetimer = 0 ;
    }
  }

void Scroller::start_movetimer(void)
/*--------------------------------*/
{
  if (m_movetimer) {
    killTimer(m_movetimer) ;
    }
  m_movetimer = startTimer(100) ;  // 100ms
  }

void Scroller::timerEvent(QTimerEvent *event)
/*-----------------------------------------*/
{
  if (m_movetimer) {
    stop_movetimer() ;
    move_plot(m_newstart) ;
    }
  }

void Scroller::slider_moved(void)
/*-----------------------------*/
{
  QScrollBar *sb = m_ui.segment ;
  float duration = (float)m_recording->duration() ;
  int width = sb->maximum() + sb->pageStep() - sb->minimum() ;
  m_newstart = sb->value()*duration/(float)width ;
  show_slidertime(m_newstart) ;

  if (m_ui.segment->isSliderDown()) {
    start_movetimer() ;
    m_sliding = true ;
    }
  else if (m_sliding) {
    if (m_movetimer) {
      stop_movetimer() ;
      move_plot(m_newstart) ;
      }
    m_sliding = false ;
    }
  else {
    move_plot(m_newstart) ;
    }
  }

void Scroller::move_plot(float start)
/*---------------------------------*/
{
  if (start != m_start) {
    emit show_signals(m_recording->new_interval(start, m_duration)) ;
    emit set_plot_timerange(start, m_duration) ;
    m_start = start ;
    emit show_annotations() ;
    }
  }

void Scroller::on_segment_valueChanged(int position)
/*------------------------------------------------*/
{
  slider_moved() ;
  // Sluggish if large data segments with tracking...
  // Tracking is on, show time at slider position
  // But also catch slider released and use this to refresh chart data...
  }

void Scroller::on_segment_sliderReleased(void)
/*------------------------------------------*/
{
  slider_moved() ;
  }

