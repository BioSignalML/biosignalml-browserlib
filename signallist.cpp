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

#include "signallist.h"

using namespace browser ;


QString browser::signal_uri(bsml::Signal::Ptr signal)
/*=================================================*/
{
  QString prefix = ((std::string)(signal->recording())).c_str() ;
  QString uri = ((std::string)signal->uri()).c_str() ;
  if (uri.startsWith(prefix)) return uri.mid(prefix.size()) ;
  else                        return uri ;
  }


static const QStringList HEADER{"", "Label", "Uri"} ;
static const int         ID_COLUMN = 2 ;               // Uri is ID


SignalModel::SignalModel(QObject *parent, bsml::Recording::Ptr recording)
/*---------------------------------------------------------------------*/
: QAbstractTableModel(parent),
  m_rows(SigList())
{
  for (auto const &u : recording->get_signal_uris()) {
    auto s = recording->get_signal(u) ;
    m_rows.append(SigInfo(true, QVariant(QString(s->label().c_str())), QVariant(signal_uri(s)))) ;
    }
  }

int SignalModel::rowCount(const QModelIndex &index) const
/*-----------------------------------------------------*/
{
  return m_rows.size() ;
  }

int SignalModel::columnCount(const QModelIndex &index) const
/*--------------------------------------------------------*/
{
  return HEADER.size() ;
  }

QVariant SignalModel::headerData(int section, Qt::Orientation orientation, int role) const
/*--------------------------------------------------------------------------------------*/
{
  if (orientation == Qt::Horizontal) {
    if (role == Qt::DisplayRole) {
      return HEADER[section] ;
      }
    else if (role == Qt::TextAlignmentRole) {
      return Qt::AlignLeft ;
      }
    else if (role == Qt::FontRole) {
      QFont font = QFont(QApplication::font()) ;
      font.setBold(true) ;
      return font ;
      }
    }
  return QVariant() ;
  }

QVariant SignalModel::data(const QModelIndex &index, int role) const
/*----------------------------------------------------------------*/
{
  if (role == Qt::DisplayRole) {
    if      (index.column() == 1) return std::get<1>(m_rows[index.row()]) ;
    else if (index.column() == 2) return std::get<2>(m_rows[index.row()]) ;
    }
  else if (role == Qt::CheckStateRole) {
    if (index.column() == 0)
      return std::get<0>(m_rows[index.row()]) ? Qt::Checked : Qt::Unchecked ;
    }
  return QVariant() ;
  }

bool SignalModel::setData(const QModelIndex &index, const QVariant &value, int role)
/*--------------------------------------------------------------------------------*/
{
  if (role == Qt::CheckStateRole && index.column() == 0) {
    bool checked = (value == Qt::Checked) ;
    std::get<0>(m_rows[index.row()]) = checked ;
    emit rowVisible(std::get<ID_COLUMN>(m_rows[index.row()]).toString(), checked) ;
    emit dataChanged(index, index) ;
    return true ;
    }
  return false ;
  }

Qt::ItemFlags SignalModel::flags(const QModelIndex &index) const
/*------------------------------------------------------------*/
{
  return (index.column() == 0) ? Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable
                               : Qt::ItemIsEnabled | Qt::ItemIsSelectable ;
  }

void SignalModel::setVisibility(bool visible)
/*-----------------------------------------*/
{
  for (auto r = 0 ;  r < m_rows.size() ;  ++r)
    setData(createIndex(r, 0), visible ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole) ;
  }

bool SignalModel::move_rows(int from, const QModelIndex &index)
/*-----------------------------------------------------------*/
{
  int to = index.row() ;
  if (from == to) return false ;
  if (beginMoveRows(index, from, from, index, (from > to) ? to : to + 1)) {
    QString from_id = std::get<ID_COLUMN>(m_rows[from]).toString() ;
    QString to_id   = std::get<ID_COLUMN>(m_rows[to]).toString() ;
    m_rows.move(from, to) ;
    emit rowMoved(from_id, to_id) ;
    endMoveRows() ;
    return true ;
    }
  return false ;
  }


SignalList::SignalList(QWidget *parent, bsml::Recording::Ptr recording) //, annotator
/*===================================================================*/
: QWidget(parent),
  m_recording(recording),
  m_model(new SignalModel(parent, recording)),
// m_annotator(annotator),
  m_ui(Ui::SignalView())
{
  m_ui.setupUi(this) ;
  m_ui.signallist->setModel(m_model) ;
  m_ui.signallist->setColumnWidth(0, 25) ;

  // Connect m_model.moveRow slot and m_ui.signallist.?? signal
  }

void SignalList::plot_signals(float start, float duration)
/*------------------------------------------------------*/
{
  auto interval = bsml::Interval::create(rdf::URI(), start, duration) ;
  for (auto const &u : m_recording->get_signal_uris()) {
    auto s = m_recording->get_signal(u) ;
    QString uri = signal_uri(s) ;
// TODO    if (s->units() == uom::UNITS::AnnotationData.uri()) {
// TODO      emit add_event_trace(uri, s->label().c_str(), m_annotator) ;
// TODO      }
// TODO    else {
      QString units ;
// TODO      try {
// TODO        units = uom::RESOURCES[s->units()].label().c_str() ;
// TODO        }
// TODO      catch {
        QString unit = ((std::string)s->units()).c_str() ;
        units = unit.mid(unit.indexOf('#')+1) ;
// TODO        }
      emit add_signal_trace(uri, s->label().c_str(), units, true) ;
      // , ymin=s.minValue, ymax=s.maxValue)
// TODO      }
    }
  emit show_signals(interval) ;
  }


void SignalList::on_allsignals_toggled(bool state)
/*----------------------------------------------*/
{
  m_model->setVisibility(state) ;
  }
