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

#ifndef BROWSER_WIDGETS_H
#define BROWSER_WIDGETS_H

#include <QFrame>


namespace browser {

  class ChartFrame : public QFrame
  /*============================*/
  {
   Q_OBJECT

   public:
    ChartFrame(QWidget *parent) ;

    void resizeEvent(QEvent *event) ;

   signals:
    /**
     * Emit a signal when the size of the chart's frame changes.
     */
    void frameResize(QRect) ;
    } ;

  } ;

#endif
