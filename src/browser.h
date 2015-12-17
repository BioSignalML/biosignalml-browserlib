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

#ifndef BROWSER_BROWSER_H
#define BROWSER_BROWSER_H

#include "browser_exports.h"
#include "typedefs.h"

#include <biosignalml/biosignalml.h>

#include <QMainWindow>
#include <QShortcut>


namespace Ui {
  class MainWindow ;  // Declare forward
  } ;

namespace browser {

  // Forward declarations
  class SignalList ;
  class AnnotationList ;
  class Scroller ;
  class SignalReadThread ;

  class BROWSER_EXPORT Browser : public QMainWindow
  /*=============================================*/
  {
   Q_OBJECT

   public:
    Browser(bsml::Recording::Ptr recording, float start=0.0, float end=NAN,
      StringDictionary semantic_tags=StringDictionary()) ;  // annotator=None) ; LAMBDA
    ~Browser() ;

    void exportRecording(const QString &filename, float start, float end) ;
    void closeEvent(QCloseEvent *event) ;

   public slots:
    void plot_signals(bsml::Interval::Ptr interval) ;
    void set_modified(const rdf::URI &uri) ;

   signals:
    void reset_annotations(void) ;
//  void resize_annotation_list(void) ;
//  void show_slider_time(float) ;

   private:
    void stop_readers(void) ;

    Ui::MainWindow *m_ui ;
    bsml::Recording::Ptr m_recording ;
    bool m_modified ;
    QShortcut *m_closekey ;
    QList<SignalReadThread  *> m_readers ;
    float m_start ;

    SignalList *m_signals ;
    AnnotationList *m_annotations ;
    Scroller *m_scroller ;
    } ;

  } ;

#endif
