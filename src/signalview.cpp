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

#include "signalview.h"
#include "signallist.h"

#include "ui_signalview.h"

#include <QPainter>
#include <QMouseEvent>
#include <QtDebug>

using namespace browser ;


SignalItem::SignalItem(QObject *parent)
/*===================================*/
: QStyledItemDelegate(parent)
{
  }

void SignalItem::paint(QPainter *painter, const QStyleOptionViewItem &option,
/*-------------------------------------------------------------------------*/
                       const QModelIndex &index) const
{
  const QRect r = option.rect ;
  QStyleOptionViewItem viewoption(option) ;
  if (index.column() == 0) {
    int margin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1 ;
    viewoption.rect = QStyle::alignedRect(
      option.direction, Qt::AlignCenter,
      QSize(option.decorationSize.width() + 5, option.decorationSize.height()),
      QRect(r.x() + margin, r.y(), r.width() - (2 * margin), r.height())
      ) ;
    }
  painter->drawLine(r.x(), r.y() + r.height(), r.x() + r.width(), r.y() + r.height()) ;
  QStyledItemDelegate::paint(painter, viewoption, index) ;
  }


bool SignalItem::editorEvent(QEvent *event, QAbstractItemModel *model,
/*------------------------------------------------------------------*/
                             const QStyleOptionViewItem &option, const QModelIndex &index)
{
  Qt::ItemFlags flags = model->flags(index) ;
  if (!((flags & (Qt::ItemIsUserCheckable | Qt::ItemIsEnabled))
      && index.isValid()
      && event->type() == QEvent::MouseButtonRelease))
    return false ;
  int margin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1 ;
  const QRect r = option.rect ;
  auto mouseevent = static_cast<QMouseEvent *>(event) ;
  if (!QStyle::alignedRect(
         option.direction, Qt::AlignCenter,
         option.decorationSize,
         QRect(r.x() + (2 * margin), r.y(), r.width() - (2 * margin), r.height())
         ).contains(mouseevent->pos()))
    return false ;

  return model->setData(index,
                        (index.data(Qt::CheckStateRole) == Qt::Checked) ? Qt::Unchecked
                                                                        : Qt::Checked,
                        Qt::CheckStateRole) ;
  }


SignalView::SignalView(QWidget *parent)
/*===================================*/
: QTableView(parent), m_selectedrow(-1)
{
  setItemDelegate(new SignalItem(this)) ;
  setShowGrid(false) ;
  verticalHeader()->hide() ;
  horizontalHeader()->setStretchLastSection(true) ;
  horizontalHeader()->setHighlightSections(false) ;
  setSelectionMode(QAbstractItemView::SingleSelection) ;
  setSelectionBehavior(QAbstractItemView::SelectRows) ;
  }


void SignalView::selectionChanged(const QItemSelection &selected,
/*-------------------------------------------------------------*/
                                  const QItemSelection &deselected)
{
  QTableView::selectionChanged(selected, deselected) ;
  for (auto const index : selected.indexes()) {
    if (m_selectedrow != index.row()) {
      m_selectedrow = index.row() ;
      emit rowSelected(m_selectedrow) ;
      }
    }
  for (auto const index : deselected.indexes()) {
    if (m_selectedrow == index.row()) {
      m_selectedrow = -1 ;
      emit rowSelected(m_selectedrow) ;
      }
    }
  }

void SignalView::mousePressEvent(QMouseEvent *event)
/*------------------------------------------------*/
{
  int row = rowAt(event->pos().y()) ;
  if (row >= 0) selectRow(row) ;
  else          clearSelection() ;
  update() ;
  }

void SignalView::mouseMoveEvent(QMouseEvent *event)
/*-----------------------------------------------*/
{
  int row = rowAt(event->pos().y()) ;
  if (m_selectedrow >= 0 && row >= 0) {
    if (static_cast<SignalModel *>(model())->move_rows(m_selectedrow, indexAt(event->pos()))) {
      selectRow(row) ;
      update() ;
      }
    }
  }

void SignalView::mouseReleaseEvent(QMouseEvent *event)
/*--------------------------------------------------*/
{
  QTableView::mouseReleaseEvent(event) ;
  }
