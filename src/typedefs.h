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

#ifndef BROWSER_TYPEDEFS_H
#define BROWSER_TYPEDEFS_H

#include <QPair>
#include <QHash>
#include <QList>
#include <QMap>
#include <QRect>
#include <QStringList>
#include <QVariant>

#include <tuple>
#include <functional>


namespace browser {

  class Trace ;          // Declare forward

  //! A row of a table
  using RowData = QVector<QVariant> ;
  //! `first` and `last` position of rows added to a table
  using RowPosns = QPair<int, int> ;
  //! Get data for a cell in a table
  using TableDataFunction = std::function<QVariant(int, int)> ;

  using EventMap = std::function<QString (float)> ;
  using EventInfo = QPair<float, QString> ;
  using EventPosInfo = std::tuple<float, float, QString> ;

  using QStringPair = std::pair<QString, QString> ;
  using QStringDictionary = QHash<QString, QString> ;
  using floatPair = std::pair<float, float> ;

  using TraceInfo = std::tuple<QString, bool, std::shared_ptr<Trace>> ;
  using TraceList = QList<TraceInfo> ;

  using PosnTime = QPair<int, float> ;

  //! <start, end, text, tags, editable>
  using AnnInfo = std::tuple<float, float, QString, QStringList, bool> ;
  using AnnotationDict = QMap<QString, AnnInfo> ;

  using AnnRectList = QList<QPair<QRect, QString>> ; // List of tuple(rect, id)
  } ;

#endif
