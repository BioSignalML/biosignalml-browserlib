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

#ifndef BROWSER_ANNOTATIONLIST_H
#define BROWSER_ANNOTATIONLIST_H

#include "typedefs.h"
#include "table.h"
#include "nrange.h"

#include "ui_annotationlist.h"

#include <biosignalml/biosignalml.h>

#include <QWidget>

#include <memory>


namespace browser {

  typedef std::tuple<bsml::Annotation, RowData, bool> AnnRow ;
  typedef std::unique_ptr<const AnnRow> AnnRowPtr ;

  class AnnotationTable : public TableModel::TableModel
  /*=================================================*/
  {
    // <annotation, rowdata, editable>
    typedef QList<AnnRow> AnnRows ;

   public:
    AnnotationTable(const NumericRange &timemap) ;

    AnnRowPtr get_row(int row) const ;
    AnnRowPtr find_annotation(const QString &uri) const ;
    void add_row(const bsml::Annotation &annotation, float start, float end, const QString &type,
                 const QString &text, const QString &tagtext="", bool editable=false) ;
    void delete_row(const QString &uri) ;

    static QStringList header(void) ;

    QVariant data(const QModelIndex &index, int role) const ;

   private:
    AnnRows m_rows ;
    NumericRange m_timemap ;
    } ;


  class AnnotationList : public QWidget
  /*=================================*/
  {
    Q_OBJECT

    typedef QHash<QString, floatPair> EventDict ;  //!< uri: (start, duration)

   public:
    AnnotationList(QWidget *parent, const bsml::Recording &recording,
                   const QStringDictionary &semantic_tags) ;
    ~AnnotationList() ;

   public slots:
    void show_annotations(void) ;

    void on_annotations_doubleClicked(const QModelIndex &index) ;
    void on_events_currentIndexChanged(const QString &eventtype) ;
    void add_annotation(float start, float end, const QString &text, const QStringList &tags,
                        const bsml::Annotation &predecessor=bsml::Annotation()) ;
    void modify_annotation(const QString &id, const QString &text, const QStringList &tags) ;
    void delete_annotation(const QString &id) ;

   signals:
    void annotationAdded(const QString &, float, float, const QString &, const QStringList &, bool) ;
    void annotationDeleted(const QString &) ;
    void move_plot(float) ;
    void set_marker(float) ;
    void set_slider_value(float) ;
    void show_slider_time(float) ;
    void modified(const rdf::URI &uri) ;

   private:
    QString tag_labels(const QStringList &tags) ;

    void append_annotation(bsml::Object::Reference about, const QString &text,
                           const QStringList &tags,
                           const bsml::Annotation &predecessor=bsml::Annotation()) ;
    void remove_annotation(const QString &id) ;

    bsml::Recording m_recording ;
    QStringDictionary m_semantic_tags ;
    Ui_AnnotationList m_ui ;

    SortedTable *m_table ;
    AnnotationTable *m_tablerows ;

    EventDict m_events ;
    RowPosns m_event_posns ;

    bool m_settingup ;
    } ;

  } ;

#endif
