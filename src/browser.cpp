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

#include "typedefs.h"
#include "mainwindow.h"

#include <biosignalml/data/hdf5.h>

#include <QApplication>

#include <vector>
#include <cmath>
#include <iostream>
#include <exception>

using namespace browser ;


int main(int argc, char *argv[])
/*============================*/
{
#if TESTING
  QApplication app(argc, argv) ;

  ChartPlot chart ;
  chart.addSignalTrace("1", "Sine wave", "units 1") ;
  chart.addSignalTrace("2", "Cosine",    "units 2") ;

  int points = 1000 ;
  std::vector<double> sine(points+1) ;
  std::vector<double> cosine(points+1) ;
  for (int n = 0 ;  n <= points ;  ++n) {
    sine[n] = std::sin(2.0*M_PI*n/(double)points) ;
    cosine[n] = std::cos(2.0*M_PI*n/(double)points) ;
    }
//  print(tsdata) ;
  auto sindata = std::make_shared<bsml::data::UniformTimeSeries>(1, sine) ;
  auto cosdata = std::make_shared<bsml::data::UniformTimeSeries>(1, cosine) ;
  chart.setTimeRange(0.0, points) ;
  chart.appendData("1", sindata) ;
  chart.appendData("2", cosdata) ;

  chart.addAnnotation("ann1", 250, 750, "the big middle bit...", QStringList(), true) ;
  chart.addAnnotation("ann2", 450, 550, "the little middle bit...", QStringList{"tag1", "tag2"}, true) ;

  chart.show() ;
  return app.exec() ;
#else
  if (argc <= 1) {
    std::cerr << "Usage: "<< argv[0] << " RECORDING [start] [duration]" << std::endl ;
    exit(1) ;
    }

  QString uri(argv[1]) ;

  float start = 0.0 ;
  float end = NAN ;
  bool ok = false ;
  if (argc >= 3) {
    start = QString(argv[2]).toFloat(&ok) ;
    if (!ok) {
      std::cerr << "Invalid start time" << std::endl ;
      exit(1) ;
      }
    }
  if (argc >= 4) {
    end = QString(argv[3]).toFloat(&ok) ;
    if (!ok) {
      std::cerr << "Invalid duration" << std::endl ;
      exit(1) ;
      }
    }

  QApplication app(argc, argv) ;
  app.setStyle("fusion") ;   //# For Ubuntu 14.04

  auto semantic_tags = QStringDictionary{} ;
  bsml::HDF5::Recording::Ptr hdf5 = nullptr ;
  try {
    if (uri.startsWith("http://")) {
//TODO      store = biosignalml.client.Repository(uri) ;
//TODO      recording = store.get_recording(uri) ;
//TODO      semantic_tags = store.get_semantic_tags() ;
      }
    else {                        //open ??
      hdf5 = bsml::HDF5::Recording::create(uri.toStdString(), false) ;  // Open for reading, read/write
      semantic_tags = QStringDictionary {  // Load from file
          {"http://standards/org/ontology#tag1", "Tag 1"},
          {"http://standards/org/ontology#tag2", "Tag 2"},
          {"http://standards/org/ontology#tag3", "Tag 3"},
          {"http://standards/org/ontology#tag4", "Tag 4"}
        } ;
      }
    }
  catch (std::exception &e) {
    //throw ;  //##################
    qCritical("Exception: %s", e.what()) ;
    exit(1) ;
    }

  MainWindow viewer(hdf5, start, end, semantic_tags) ; //TODO, annotator=wfdbAnnotation) ;
  viewer.show() ;

  return app.exec() ;
#endif
  }
