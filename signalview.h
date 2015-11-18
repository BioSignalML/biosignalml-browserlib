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

#ifndef BROWSER_SIGNALVIEW_H
#define BROWSER_SIGNALVIEW_H

#include <QStyledItemDelegate>
#include <QTableView>


namespace browser {

  class SignalItem : public QStyledItemDelegate
  /*=========================================*/
  {
   public:
    SignalItem(QObject *parent = nullptr) ;

    /**
     *  Paint a centered checkbox in the first column of the table
     *  and draw a line underneath each table row,
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const ;

    /**
        Toggle checkbox on mouse click and key press.
     */
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) ;
    } ;


  class SignalView : public QTableView
  /*================================*/
  {
   Q_OBJECT

   public:
    SignalView(QWidget *parent) ;
    void mousePressEvent(QMouseEvent *event) ;
    void mouseMoveEvent(QMouseEvent *event) ;
    void mouseReleaseEvent(QMouseEvent *event) ;

   public slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) ;

   signals:
    void rowSelected(int) ;   // row, -1 means clear

   private:
    int m_selectedrow ;
    } ;

  } ;

#endif
