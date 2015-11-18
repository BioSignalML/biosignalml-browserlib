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

#ifndef BROWSER_CHARTFORM_H
#define BROWSER_CHARTFORM_H

#include <QWidget>

#include "ui_chart.h"


namespace browser {

  class ChartForm : public QWidget
  /*============================*/
  {
   Q_OBJECT

   public:
    ChartForm(QWidget *parent) ;

    void setTimeRange(float start, float duration) ;
    void on_timescroll_valueChanged(int position) ;
    void on_frame_frameResize(QRect &geometry) ;
    void on_chart_customContextMenuRequested(const QPoint &pos) ;
    const Ui::Chart &ui(void) const { return m_ui ; }
     
   public slots:
    void position_timescroll(bool visible) ;
    void chart_resize(int offset, int width, int bottom) ;

   private:
    Ui::Chart m_ui ;
    } ;

  } ;

#endif
