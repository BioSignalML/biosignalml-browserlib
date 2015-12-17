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

#include "table.h"

#include <QHeaderView>
#include <QApplication>

using namespace browser ;


TableView::TableView(QWidget *parent)
/*=================================*/
: QTableView(parent)
{
  setAlternatingRowColors(true) ;
  setShowGrid(false) ;
  setWordWrap(true) ;
  verticalHeader()->setVisible(false) ;
  verticalHeader()->sectionResizeMode(QHeaderView::Fixed) ;
  verticalHeader()->setDefaultSectionSize(18) ;        // Magic constant...
  horizontalHeader()->setStretchLastSection(true) ;
  horizontalHeader()->setHighlightSections(false) ;
  horizontalHeader()->setSortIndicatorShown(true) ;
  setSelectionMode(QAbstractItemView::SingleSelection) ;
  setSelectionBehavior(QAbstractItemView::SelectRows) ;
  }

void TableView::resizeCells(void)  // Needs to be done after table is populated
/*-----------------------------*/
{
  QModelIndexList selected = selectedIndexes() ;
  hide() ;
  resizeColumnsToContents() ;
  show() ;
  hide() ;
  resizeRowsToContents() ;
  if (selected.size() > 0) selectRow(selected[0].row()) ;
  show() ;
  }


TableModel::TableModel(QObject *parent, const QStringList &header, const QStringList &rowids)
/*=========================================================================================*/
: QAbstractTableModel(parent),
  m_header(header),
  m_rowids(rowids),
  m_keys(QHash<QString, int>())
{
  set_keys() ;
  }
  
void TableModel::set_keys(void)
/*---------------------------*/
{    
  m_keys.clear() ;
  int n = 0 ;
  for (auto const &r : m_rowids) {
    m_keys[r] = n ;
    n += 1 ;
    }
  }

int TableModel::rowCount(const QModelIndex &parent) const
/*-----------------------------------------------------*/
{
  return m_rowids.size() ;
  }

int TableModel::columnCount(const QModelIndex &parent) const
/*--------------------------------------------------------*/
{
  return m_header.size() ;
  }

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
/*-------------------------------------------------------------------------------------*/
{
  if (orientation == Qt::Horizontal) {
    if (role == Qt::DisplayRole) {
      return m_header[section] ;
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

QVariant TableModel::data(const QModelIndex &index, int role) const
/*---------------------------------------------------------------*/
{
  return (role == Qt::TextAlignmentRole) ? Qt::AlignTop : QVariant() ;
  }

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
/*-----------------------------------------------------------*/
{
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable ;
  }

RowPosns TableModel::appendRows(const QStringList &rowids)
/*------------------------------------------------------*/
{
  RowPosns posns(m_rowids.size(), m_rowids.size() + rowids.size() - 1) ;
  beginInsertRows(createIndex(m_rowids.size(), 0), posns.first, posns.second) ;
  m_rowids.append(rowids) ;
  set_keys() ;
  endInsertRows() ;
  return posns ;
  }

void TableModel::removeRows(const RowPosns &posns)
/*----------------------------------------------*/
{
  beginRemoveRows(createIndex(posns.first, 0), posns.first, posns.second) ;
  m_rowids.erase(m_rowids.begin()+posns.first, m_rowids.begin()+posns.second+1) ;
  set_keys() ;
  endRemoveRows() ;
  }

void TableModel::deleteRow(const QString &key)
/*------------------------------------------*/
{
  int n = m_keys.value(key, -1) ;
  if (n >= 0) removeRows(RowPosns(n, n)) ;
  }


SortedTable::SortedTable(QObject *parent, TableView *view, TableModel *model)
/*=========================================================================*/
: QSortFilterProxyModel(parent),
  m_model(model)
{
  setSourceModel(m_model) ;
  view->setModel(this) ;
  view->setSortingEnabled(true) ;
  view->setColumnHidden(0, true) ;
  sort(1, Qt::AscendingOrder) ;
  view->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder) ;
  }

RowPosns SortedTable::appendRows(const QStringList &rowids)
/*-------------------------------------------------------*/
{
  emit m_model->layoutAboutToBeChanged() ;
  RowPosns posns = m_model->appendRows(rowids) ;
  emit m_model->layoutChanged() ;
  return posns ;
  }

void SortedTable::removeRows(const RowPosns &posns)
/*-----------------------------------------------*/
{
  emit m_model->layoutAboutToBeChanged() ;
  m_model->removeRows(posns) ;
  emit m_model->layoutChanged() ;
  }

void SortedTable::deleteRow(const QString &key)
/*-------------------------------------------*/
{
  emit m_model->layoutAboutToBeChanged() ;
  m_model->deleteRow(key) ;
  emit m_model->layoutChanged() ;
  }
