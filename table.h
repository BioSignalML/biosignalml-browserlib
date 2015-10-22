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

#ifndef BROWSER_TABLE_H
#define BROWSER_TABLE_H

#include "typedefs.h"

#include <QTableView>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

/**
 * A generic widget and model for working with a table view.
 *
 * Selection by row and sortable columns are provided.
 */


namespace browser {

  /**
   * A generic table view.
   */
  class TableView : public QTableView
  /*===============================*/
  {
    Q_OBJECT
   public:
    TableView(QWidget *parent) ;
    void resizeCells(void) ;
    } ;


  /**
   * A generic table model.
   *
   * :param header (list): A list of column headings.
   * :param rowids (list): A list of table data rows, with each element
   *    a list of the row's column data. The first column is used as
   *    a row identifier and is normally hidden.
   */
  class TableModel : public QAbstractTableModel
  /*=========================================*/
  {
    Q_OBJECT
   public:
    TableModel() ;
    TableModel(QObject *parent, const QStringList &header, const QStringList &rowids) ;

    int rowCount(const QModelIndex &parent=QModelIndex()) const ;
    int columnCount(const QModelIndex &parent=QModelIndex()) const ;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const ;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const ;
    Qt::ItemFlags flags(const QModelIndex &index) const ;

    RowPosns appendRows(const QStringList &rowids) ;
    void removeRows(const RowPosns &posns) ;
    void deleteRow(const QString &key) ;

   private:
    void set_keys(void) ;

    QStringList m_header ;
    QStringList m_rowids ;
    QHash<QString, int> m_keys ;
    } ;


  /**
   * A generic sorted table.
   *
   * :param view: A :class:`TableView` in which the model is displayed.
   * :param header (list): A list of column headings.
   * :param rowids (list): A list of table data rows, with each element
   *    a list of the row's column data. The first column is used as
   *    a row identifier and is hidden.
   *
   * The initial view of the model is sorted on the second column (i.e. on
   * the first visible column).
   */
  class SortedTable : public QSortFilterProxyModel
  /*============================================*/
  {
    Q_OBJECT
   public:
    SortedTable(QObject *parent, TableView *view, TableModel::TableModel *table,
                const QStringList &header, const QStringList &rowids) ;

    RowPosns appendRows(const QStringList &rowids) ;
    void removeRows(const RowPosns &posns) ;
    void deleteRow(const QString &key) ;

   private:
    TableModel *m_table ;
    } ;

  } ;

#endif
