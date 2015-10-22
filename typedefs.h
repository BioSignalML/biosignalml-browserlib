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

#include <tuple>
#include <functional>


namespace browser {

  class Trace ;          // Declare forward

  //! A row of a table
  typedef QVector<QVariant> RowData ;
  //! `first` and `last` position of rows added to a table
  typedef QPair<int, int> RowPosns ;
  //! Get data for a cell in a table
  typedef std::function<QVariant(int, int)> TableDataFunction ;

  typedef std::function<QString (float)> EventMap ;
  typedef QPair<float, QString> EventInfo ;
  typedef std::tuple<float, float, QString> EventPosInfo ;

  typedef std::pair<QString, QString> QStringPair ;
  typedef QHash<QString, QString> QStringDictionary ;
  typedef std::pair<float, float> floatPair ;

  typedef std::tuple<QString, bool, std::shared_ptr<Trace>> TraceInfo ;
  typedef QList<TraceInfo> TraceList ;

  typedef QPair<int, float> PosnTime ;

  //! <start, end, text, tags, editable>
  typedef std::tuple<float, float, QString, QStringList, bool> AnnInfo ;
  typedef QMap<QString, AnnInfo> AnnotationDict ;

  typedef QList<QPair<QRect, QString>> AnnRectList ; // List of tuple(rect, id)
  } ;

#endif
