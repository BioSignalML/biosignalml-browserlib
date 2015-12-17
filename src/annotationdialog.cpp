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

#include "annotationdialog.h"

using namespace browser ;


TagItem::TagItem(const QString &uri, const QString &label)
/*------------------------------------------------------*/
: QListWidgetItem(label), m_uri(uri), m_label(label)
{
  }


AnnotationDialog::AnnotationDialog(ChartPlot *parent, const QString &id, float start, float end,
/*--------------------------------------------------------------------------------------------*/
                                   const QString &text, const QStringList &tags)
: QDialog(parent),
  m_ui(Ui_AnnotationDialog()),
  m_tagsvisible(false),
  m_tagitems(QList<TagItem *>())
{
  m_ui.setupUi(this) ;

  QString title = id ;
  if (id.startsWith("http://") || id.startsWith("file://")) {
    QStringList p = id.mid(7).split('/') ;
    if (p.size() > 1) {
      p.removeFirst() ;
      title = p.join('/') ;
      }
    }
  setWindowTitle(title) ;

  m_ui.description->setText(QString("Annotate %1 to %2 seconds").arg(start).arg(end)) ;
  m_ui.annotation->setPlainText(text) ;
  m_ui.taglist->setSelectionMode(QAbstractItemView::ExtendedSelection) ;

  const StringDictionary &semantic_tags = parent->semanticTags() ;  // { uri: label }
  for (auto const &u : semantic_tags.keys())
    add_tagitem(u, semantic_tags.value(u)) ;
  for (auto const &t : tags)     // Show 'unknown' tags
    if (!semantic_tags.contains(t)) add_tagitem(t, t) ;
  m_ui.taglist->sortItems() ;
  // Setting selected when items are added doesn't work (because of sort??)
  for (int n = 0 ;  n < m_ui.taglist->count() ;  ++n) {
    auto const &t = m_ui.taglist->item(n) ;
    if (tags.contains(dynamic_cast<TagItem *>(t)->uri())) t->setSelected(true) ;
    }
  m_ui.taglist->hide() ;
  QObject::connect(m_ui.tags, SIGNAL(clicked()), this, SLOT(show_tags())) ;
  QStringList tagnames ;
  for (auto const &t : tags)
    tagnames.append(semantic_tags.value(t, t)) ;
  std::sort(tagnames.begin(), tagnames.end()) ;
  m_ui.taglabels->setText(tagnames.join(", ")) ;
  }

AnnotationDialog::~AnnotationDialog()
/*---------------------------------*/
{
  for (auto const &t : m_tagitems) delete t ;
  }

QString AnnotationDialog::get_annotation(void) const
/*------------------------------------------------*/
{
  return m_ui.annotation->toPlainText().trimmed() ;
  }

void AnnotationDialog::show_tags(void)
/*----------------------------------*/
{
  QStringList labels ;
  for (auto const &t : m_ui.taglist->selectedItems())
    labels.append(dynamic_cast<TagItem *>(t)->label()) ;
  std::sort(labels.begin(), labels.end()) ;
  m_ui.taglabels->setText(labels.join(", ")) ;
  m_tagsvisible = !m_tagsvisible ;
  m_ui.taglist->setVisible(m_tagsvisible) ;
  }

QStringList AnnotationDialog::get_tags(void) const
/*----------------------------------------------*/
{
  QStringList tags ;
  for (auto const &t : m_ui.taglist->selectedItems())
    tags.append(dynamic_cast<TagItem *>(t)->uri()) ;
  return tags ;
  }

void AnnotationDialog::add_tagitem(const QString &uri, const QString &label)
/*------------------------------------------------------------------------*/
{
  TagItem *item = new TagItem(uri, label) ;
  m_ui.taglist->addItem(item) ;
  m_tagitems.append(item) ;
  }
