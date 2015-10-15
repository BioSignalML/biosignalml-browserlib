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

#include "chartplot.h"

#include <QApplication>

#include <vector>
#include <cmath>

using namespace browser ;


int main(int argc, char *argv[])
/*============================*/
{
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
  chart.show() ;

  return app.exec() ;
  }
