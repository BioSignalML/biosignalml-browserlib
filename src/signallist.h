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

#ifndef BROWSER_SIGNALLIST_H
#define BROWSER_SIGNALLIST_H

#include "typedefs.h"

#include "ui_signalview.h"

#include <biosignalml/biosignalml.h>

#include <QWidget>
#include <QAbstractTableModel>


namespace browser {

  QString signal_uri(bsml::Signal::Ptr signal) ;


  class SignalModel : public QAbstractTableModel
  /*==========================================*/
  {
   Q_OBJECT

   public:
    SignalModel(QObject *parent, bsml::Recording::Ptr recording) ;

    int rowCount(const QModelIndex &parent=QModelIndex()) const ;
    int columnCount(const QModelIndex &parent=QModelIndex()) const ;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const ;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const ;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) ;
//    bool moveRow(const QModelIndex &, int, const QModelIndex &, int) ;
    Qt::ItemFlags flags(const QModelIndex &index) const ;

    void setVisibility(bool visible) ;
    bool move_rows(int from, const QModelIndex &index) ;

   signals:
    void rowVisible(QString, bool) ;   // id, state
    void rowMoved(QString, QString) ;  // from_id, to_id

   private:
    typedef std::tuple<bool, QVariant, QVariant> SigInfo ;
    typedef QList<SigInfo> SigList ;

    SigList m_rows ;
    } ;


  class SignalList : public QWidget
  /*=============================*/
  {
   Q_OBJECT

   public:
    SignalList(QWidget *parent, bsml::Recording::Ptr recording) ; //, annotator) ;

    void plot_signals(float start, float duration) ;
    const Ui::SignalView &ui(void) const { return m_ui ; }
    SignalModel *model(void) const { return m_model ; }

   signals:
    void add_event_trace(QString, QString, bool) ; // ****, FunctionType, DataSegment)
    void add_signal_trace(QString, QString, QString, bool) ; //# , DataSegment, float, float)
    void show_signals(bsml::Interval::Ptr) ;

   public slots:
    void on_allsignals_toggled(bool state) ;

   private:
    Ui::SignalView m_ui ;
    SignalModel *m_model ;
    bsml::Recording::Ptr m_recording ;
    } ;

  } ;

#endif
