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

#include "annotationlist.h"

using namespace browser ;


QStringDictionary PREFIXES {
  QStringPair("bsml",  "http://www.biosignalml.org/ontologies/2011/04/biosignalml#"),
  QStringPair("dct",   "http://purl.org/dc/terms/"),
  QStringPair("rdfs",  "http://www.w3.org/2000/01/rdf-schema#"),
  QStringPair("pbank", "http://www.biosignalml.org/ontologies/examples/physiobank#")
  } ;


QString abbreviate_uri(const QString &uri)
/*--------------------------------------*/
{
  for (auto const &pfx : PREFIXES)
    if (uri.startsWith(PREFIXES[pfx])) return pfx + ":" + uri.right(PREFIXES[pfx].size()) ;
  return uri ;
  }

QString expand_uri(const QString &uri)
/*----------------------------------*/
{
  for (auto const &pfx : PREFIXES)
    if (uri.startsWith(pfx + ":")) return PREFIXES[pfx] + uri.right(pfx.size()+1) ;
  return uri ;
  }


AnnotationTable::AnnotationTable(const NumericRange &timemap)
/*=========================================================*/
: m_rows(AnnRows()), m_timemap(timemap)
{
  }

QStringList AnnotationTable::header(void)
/*-------------------------------------*/
{
  return QStringList{"", "Start", "End", "Duration",  "Type", "Annotation", "Tags"} ;
  }

AnnRowPtr AnnotationTable::get_row(int row) const
/*---------------------------------------------*/
{
  if (row >=0 && row < m_rows.size())
    return AnnRowPtr(&m_rows.at(row)) ;
  return nullptr ;
  }

AnnRowPtr AnnotationTable::find_annotation(const QString &uri) const
/*----------------------------------------------------------------*/
{
  for (auto const &ann : m_rows) {
    if (uri == std::get<1>(ann)[0])
      return AnnRowPtr(&ann) ;
    }
  return nullptr ;
  }

void AnnotationTable::add_row(const bsml::Annotation &annotation, float start, float end,
/*-------------------------------------------------------------------------------------*/
                              const QString &type, const QString &text,
                              const QString &tagtext, bool editable)
{
  RowData rowdata(7) ; // <uri, start, end, duration, type, text, tagtext>
  rowdata[0] = QVariant(QString(((std::string)annotation.uri()).c_str())) ;
  if (isnan(start)) {
    rowdata[1] = QVariant(QString("")) ;
    rowdata[2] = QVariant(QString("")) ;
    rowdata[3] = QVariant(QString("")) ;
    }
  else {
    float nstart = m_timemap.map(start) ;  // Normalise for display
    rowdata[1] = QVariant(nstart) ;
    if (isnan(end)) {
      rowdata[2] = QVariant(QString("")) ;
      rowdata[3] = QVariant(QString("")) ;
      }
    else {
      float nend = m_timemap.map(end) ;
      rowdata[2] = QVariant(nend) ;
      rowdata[3] = QVariant(nend - nstart) ;
      }
    }
  rowdata[4] = QVariant(type) ;
  rowdata[5] = QVariant(text) ;
  rowdata[6] = QVariant(tagtext) ;

  m_rows.append(AnnRow(annotation, rowdata, editable)) ;
  }

//  [ AnnotationTable.row(uri,
//    m_make_ann_times(start, end),
//    'Annotation', text,
//    m_tag_labels(tags)) ]) ;

void AnnotationTable::delete_row(const QString &uri)
/*------------------------------------------------*/
{
  for (auto row = m_rows.begin() ;  row < m_rows.end() ;  ++row) {
    if (uri == std::get<1>(*row)[0]) {
      m_rows.erase(row) ;
      return ;
      }
    }
  }

QVariant AnnotationTable::data(const QModelIndex &index, int role) const
/*--------------------------------------------------------------------*/
{
  if (role == Qt::DisplayRole) {
    int row = index.row() ;
    int col = index.column() ;
    if (row >= 0 && row < m_rows.size()) {
      const RowData &rowdata = std::get<1>(m_rows.at(row)) ;
      if (col >= 0 && col < rowdata.size()) return rowdata.at(col) ;
      }
    else return QVariant() ;
    }
  return TableModel::data(index, role) ;
  }


AnnotationList::AnnotationList(QWidget *parent, const bsml::Recording &recording,
/*=============================================================================*/
                               const QStringDictionary &semantic_tags)
: QWidget(parent),
  m_recording(recording),
  m_semantic_tags(semantic_tags),
  m_ui(Ui_AnnotationList()),
  m_table(nullptr),
  m_tablerows(new AnnotationTable(NumericRange(0.0, (float)recording.duration()))),
  m_events(EventDict()),
  m_event_posns(RowPosns(-1, -1)),
  m_settingup(true)
{
  m_ui.setupUi(this) ;

  QStringList annrows ;
  for (auto const &a : m_recording.get_annotations()) {  // Or via graph() ??
    float annstart = NAN ;
    float annend = NAN ;
    if (a.time().is_valid()) {
      annstart = a.time().start() ;
      if (!isnan(a.time().duration()) && a.time().duration() != 0.0) annend   = a.time.end() ;
      }

    auto tags = a.tags() ;
// TODO     if (not isinstance(tags, list)) tags = [ tags ] ;

    m_tablerows->add_row(a, annstart, annend, a.comment(), "Annotation", tag_labels(tags), false) ;
    annrows.append((std::string)a.uri()) ;
    }


  m_table = new SortedTable(this, m_ui.annotations, m_tablerows, AnnotationTable::header(), annrows) ;

//                        [ AnnotationTable.row(a[0], m_make_ann_times(a[1], a[2]),
//                          'Annotation' if a[5] else 'Event',
//                          a[3], tag_labels(a[4])) ;
//                        for a in m_annotations ]) ;

//    for e in [m_recording.graph.get_event(evt)
//                for evt in self._recording.graph.get_event_uris(timetype=bsml::BSML::Interval)]:
//      if e.time.end is None: e.time.end = e.time.start
//      self._annotations.append( (str(e.uri), e.time.start, e.time.end, abbreviate_uri(e.eventtype), [], False, e) )

  m_ui.events->addItem("None") ;
//  TODO ********  m_ui.events->insertItems(1, ['%s (%s)' % (abbreviate_uri(etype), count)
//    for etype, count in m_recording.graph.get_event_types(counts=true)]) ;
  // if no duration ...
  m_ui.events->addItem("All") ;
  m_settingup = false ;
  }

AnnotationList::~AnnotationList()
/*-----------------------------*/
{
  if (m_table) delete m_table ;
  delete m_tablerows ;
  }

void AnnotationList::show_annotations(void)
/*--------------------------------------*/
{
// TODO ***** for a in m_annotations {  // tuple(uri, start, end, text, tags, resource)
//    if (!isnan(a[1])) emit annotationAdded(*a[6]) ;  // ????
//    }
  }

QString AnnotationList::tag_labels(const QStringList &tags)
/*-------------------------------------------------------*/
{
  QStringList result ;
  for (auto const &t : tags)
    result.append(m_semantic_tags.value(t, t)) ;
  std::sort(result.begin(), result.end()) ;
  return result.join(", ") ;
  }

void AnnotationList::on_annotations_doubleClicked(const QModelIndex &index)
/*-----------------------------------------------------------------------*/
{
  AnnRowPtr annrow = m_tablerows->get_row(index.row()) ;
  const RowData &rowdata = std::get<1>(*annrow) ;
  const QString &uri = rowdata[0].toString() ;

  float time = NAN ;
  float start = NAN ;
  float duration = NAN ;

  auto evt = m_events.value(uri, floatPair(NAN, NAN)) ;
  if (!isnan(evt.first)) {
    time = evt.first ;
    duration = evt.second ;
    }
  else {
    time = rowdata[1].toFloat() ;
    if (rowdata[2].toFloat() > 0.0) duration = rowdata[2].toFloat() - time ;
    }

  if (!isnan(time)) {
    if (!isnan(duration)) {
      start = std::max(0.0, time - duration/2.0) ;
      duration = std::min(time + duration + duration/2.0, (double)m_recording.duration())
                - start ;
      }
    else {
      start = std::max(0.0, time - duration/4.0) ;
      }
    emit move_plot(start) ;
    emit set_slider_value(start) ;
    emit show_slider_time(start) ;
    emit set_marker(time) ;
    }
  }

void AnnotationList::on_events_currentIndexChanged(const QString &eventtype)
/*------------------------------------------------------------------------*/
{
  if (m_settingup || eventtype == "") return ;  // Setting up

  if (m_event_posns.first >= 0)
    m_table->removeRows(m_event_posns) ;
  if (eventtype == "None") {
    m_event_posns = RowPosns(-1, -1) ;
    return ;
    }
  QString etype = (eventtype != "All") ? expand_uri(eventtype.mid(0, eventtype.lastIndexOf(" (")))
                                       : "" ;
  QStringList rows ;
  m_events = EventDict() ;
  for (auto const &uri : m_recording.get_event_uris(etype, bsml::BSML::Instant)) {
    const bsml::Event &event = m_recording.get_event(uri) ;
    if (event.is_valid()) {
      auto time = event.time() ;
      m_events[QString((std::string)uri)] = floatPair((float)time->start(), (float)time->duration()) ;
      rows.append(uri) ;
      float end = (float)time->start() + (float)time->duration() ;
      m_tablerows->add_row(bsml::Annotation(), (float)time->start(), end, "Event",
                           abbreviate_uri(QString(((std::string)event.eventtype()).c_str()))) ;
      }
    }
  m_event_posns = m_table->appendRows(rows) ;
  }

void AnnotationList::add_annotation(float start, float end, const QString &text,
/*----------------------------------------------------------------------------*/
                                    const QStringList &tags, const bsml::Annotation &predecessor)
{
  if (text.size() > 0 || tags.size() > 0) {
    bsml::Segment::Reference segment(m_recording.uri().make_URI(),
                                     m_recording.uri(), m_recording.interval(start, end)) ;
    m_recording.add_resource(segment) ;
    append_annotation(segment, text, tags, predecessor) ;
    emit modified(m_recording.uri()) ;
    }
  }

void AnnotationList::append_annotation(bsml::Object::Reference about, const QString &text,
/*--------------------------------------------------------------------------------------*/
                                       const QStringList &tags,
                                       const bsml::Annotation &predecessor)
{
  std::set<rdf::Node> taglist ;
  // TODO ****** add tags to taglist
  bsml::Annotation annotation(m_recording.uri().make_URI(), about, text.toStdString(),
                              taglist, predecessor) ;
  m_recording.add_resource(annotation) ;
  float start = (float)annotation.time().start() ;
  float end = start + (float)annotation.time().duration() ;

  QString uri = QString(((std::string)annotation.uri()).c_str()) ;
  m_tablerows->add_row(annotation, start, end, "Annotation", text, tag_labels(tags), true) ;
  m_table->appendRows(QStringList(uri)) ;
  emit annotationAdded(uri, start, end, text, tags, true) ;
  }

void AnnotationList::remove_annotation(const QString &id)
/*-----------------------------------------------------*/
{
  m_table->deleteRow(id) ;
  m_tablerows->delete_row(id) ;
  emit annotationDeleted(id) ;
  }

void AnnotationList::modify_annotation(const QString &uri, const QString &text,
/*---------------------------------------------------------------------------*/
                                       const QStringList &tags)
{
  AnnRowPtr annrow = m_tablerows->find_annotation(uri) ;
  if (annrow) {
    const bsml::Annotation &ann = std::get<0>(*annrow) ;
    remove_annotation(uri) ;
    if ((text.size() > 0 || tags.size() > 0) && ann.is_valid()) {
      append_annotation(ann.about(), text, tags, ann) ;
      }
    emit modified(m_recording.uri()) ;
    }
  }

void AnnotationList::delete_annotation(const QString &uri)
/*------------------------------------------------------*/
{
  remove_annotation(uri) ;
  m_recording.remove_resource(uri) ;
  emit modified(m_recording.uri()) ;
  }
