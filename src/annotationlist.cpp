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

#include <cassert>

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


AnnotationModel::AnnotationModel(QObject *parent, const NumericRange &timemap)
/*================================================--========================*/
: TableModel::TableModel(parent, this->header(), QStringList()),
  m_rows(AnnRows()), m_timemap(timemap)
{
  }

QStringList AnnotationModel::header(void)
/*-------------------------------------*/
{
  return QStringList{"", "Start", "End", "Duration",  "Type", "Annotation", "Tags"} ;
  }

const AnnRow &AnnotationModel::get_row(int row) const
/*-------------------------------------------------*/
{
  return m_rows.at(row) ;
  }

bsml::Annotation::Ptr AnnotationModel::find_annotation(const QString &uri) const
/*----------------------------------------------------------------------------*/
{
  for (auto const &row : m_rows) {
    if (uri == (*std::get<1>(row))[0])
      return std::get<0>(row) ;
    }
  return nullptr ;
  }

void AnnotationModel::add_row(bsml::Annotation::Ptr &ann, float start, float end,
/*-----------------------------------------------------------------------------*/
                              const QString &type, const QString &text,
                              const QString &tagtext, bool editable)
{
  auto rowdata = std::make_shared<RowData>(7) ; // <uri, start, end, duration, type, text, tagtext>
  RowData &data = *(rowdata.get()) ;
  // This is on local stack...
  data[0] = QVariant(QString(((std::string)ann->uri()).c_str())) ;
  if (isnan(start)) {
    data[1] = QVariant(QString("")) ;
    data[2] = QVariant(QString("")) ;
    data[3] = QVariant(QString("")) ;
    }
  else {
    float nstart = m_timemap.map(start) ;  // Normalise for display
    data[1] = QVariant(nstart) ;
    if (isnan(end)) {
      data[2] = QVariant(QString("")) ;
      data[3] = QVariant(QString("")) ;
      }
    else {
      float nend = m_timemap.map(end) ;
      data[2] = QVariant(nend) ;
      data[3] = QVariant(nend - nstart) ;
      }
    }
  data[4] = QVariant(type) ;
  data[5] = QVariant(text) ;
  data[6] = QVariant(tagtext) ;

  m_rows.append(AnnRow(ann, rowdata, editable)) ;
  }

//  [ AnnotationModel.row(uri,
//    m_make_ann_times(start, end),
//    'Annotation', text,
//    m_tag_labels(tags)) ]) ;

void AnnotationModel::delete_row(const QString &uri)
/*------------------------------------------------*/
{
  for (auto row = m_rows.begin() ;  row < m_rows.end() ;  ++row) {
    if (uri == (*std::get<1>(*row))[0]) {
      m_rows.erase(row) ;
      return ;
      }
    }
  }

QVariant AnnotationModel::data(const QModelIndex &index, int role) const
/*--------------------------------------------------------------------*/
{
  if (role == Qt::DisplayRole) {
    int row = index.row() ;
    int col = index.column() ;
    if (row >= 0 && row < m_rows.size()) {
      const RowData &rowdata = *(std::get<1>(m_rows.at(row))) ;
      if (col >= 0 && col < rowdata.size()) return rowdata.at(col) ;
      }
    else return QVariant() ;
    }
  return TableModel::data(index, role) ;
  }


AnnotationList::AnnotationList(QWidget *parent, bsml::Recording::Ptr recording,
/*===========================================================================*/
                               const QStringDictionary &semantic_tags)
: QWidget(parent),
  m_recording(recording),
  m_semantic_tags(semantic_tags),
  m_ui(Ui_AnnotationList()),
  m_model(new AnnotationModel(this, NumericRange(0.0, (float)recording->duration()))),
  m_table(nullptr),
  m_events(EventDict()),
  m_event_posns(RowPosns(-1, -1)),
  m_settingup(true)
{
  m_ui.setupUi(this) ;
  QStringList annrows ;
  for (auto const &u : m_recording->get_annotation_uris()) {  // Or via graph() ??
    auto a = m_recording->get_annotation(u) ;
    float annstart = NAN ;
    float annend = NAN ;
    auto tm = a->time() ;
    if (tm->is_valid()) {
      annstart = (float)tm->start() ;
      float d = (float)tm->duration() ;
      if (!isnan(d) && d != 0.0) annend = annstart + d ;
      }
    QStringList tags ;
    for (auto const &t : a->tags()) tags << ((std::string)t).c_str() ;
    annrows.append(((std::string)a->uri()).c_str()) ;
    m_model->add_row(a, annstart, annend, "Annotation", a->comment().c_str(), tag_labels(tags), false) ;
    }
  m_table = new SortedTable(this, m_ui.annotations, m_model) ;
  m_table->appendRows(annrows) ;

//    for e in [m_recording->get_event(evt)
//                for evt in self._recording.graph.get_event_uris(timetype=bsml::BSML::Interval)]:
//      if e.time.end is None: e.time.end = e.time.start
//      self._annotations.append( (str(e.uri), e.time.start, e.time.end, abbreviate_uri(e.eventtype), [], False, e) )

  m_ui.events->addItem("None") ;
//  TODO ********  m_ui.events->insertItems(1, ['%s (%s)' % (abbreviate_uri(etype), count)
//    for etype, count in m_recording.get_event_types(counts=true)]) ;
  // if no duration ...
  m_ui.events->addItem("All") ;
  m_settingup = false ;
  }

AnnotationList::~AnnotationList()
/*-----------------------------*/
{
  delete m_model ;
  // m_table is a QObject with a parent so doesn't need deleting
  }

void AnnotationList::show_annotations(void)
/*--------------------------------------*/
{
  for (const auto &a : m_model->rows()) {  // AnnRow(ann, rowdata, editable)
    RowData &data = *(std::get<1>(a).get()) ; // <uri, start, end, duration, type, text, tagtext>
    if (!isnan(data[1].toFloat()))
      emit annotationAdded(data[0].toString(), data[1].toFloat(), data[2].toFloat(),
                           data[5].toString(), data[6].toStringList(), std::get<2>(a)) ;
    }
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
  auto annrow = m_model->get_row(index.row()) ;
  const RowData &rowdata = *(std::get<1>(annrow)) ;
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
      duration = std::min(time + duration + duration/2.0, (double)m_recording->duration())
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
  for (auto const &u : m_recording->get_event_uris(bsml::BSML::Instant)) {
    auto event = m_recording->get_event(u) ;
    if (event->is_valid()) {
      QString uri = ((std::string)u).c_str() ;
      auto time = event->time() ;
      m_events[uri] = floatPair((float)time->start(), (float)time->duration()) ;
      rows.append(uri) ;
      float end = (float)time->start() + (float)time->duration() ;
      // Have pointers in table... (and pass nullptr here)
      auto ann = bsml::Annotation::create() ;
      m_model->add_row(ann, (float)time->start(), end, "Event",
                       abbreviate_uri(((std::string)event->eventtype()).c_str())) ;
      }
    }
  m_event_posns = m_table->appendRows(rows) ;
  }

void AnnotationList::add_annotation(float start, float end, const QString &text,
/*----------------------------------------------------------------------------*/
                                    const QStringList &tags)
{
  if (text.size() > 0 || tags.size() > 0) {
    auto segment = bsml::Segment::create(m_recording->uri().make_URI(),
                                         m_recording->uri(),
                                         m_recording->new_interval(start, end-start)) ;
    m_recording->add_resource<bsml::Segment>(segment) ;
    append_annotation(segment, text, tags) ;
    emit recording_changed(m_recording->uri()) ;
    }
  }

void AnnotationList::append_annotation(bsml::Resource::Ptr about, const QString &text,
/*----------------------------------------------------------------------------------*/
                                       const QStringList &tags,
                                       const bsml::Annotation::Ptr &predecessor)
{
  std::set<rdf::Node> taglist ;
  for (auto const tag : tags) taglist.insert(rdf::URI(tag.toStdString())) ;
  auto annotation = bsml::Annotation::create(m_recording->uri().make_URI(), about,
                                             text.toStdString(), taglist, predecessor) ;
  m_recording->add_resource<bsml::Annotation>(annotation) ;

  auto tm = annotation->time() ;
  assert(tm->is_valid()) ;
  float start = (float)tm->start() ;
  float end = start + (float)tm->duration() ;

  QString uri = QString(((std::string)annotation->uri()).c_str()) ;
  m_model->add_row(annotation, start, end, "Annotation", text, tag_labels(tags), true) ;
  m_table->appendRows(QStringList(uri)) ;
  emit annotationAdded(uri, start, end, text, tags, true) ;
  }

void AnnotationList::remove_annotation(const QString &id)
/*-----------------------------------------------------*/
{
  m_table->deleteRow(id) ;
  m_model->delete_row(id) ;
  emit annotationDeleted(id) ;
  }

void AnnotationList::modify_annotation(const QString &uri, const QString &text,
/*---------------------------------------------------------------------------*/
                                       const QStringList &tags)
{
  auto ann = m_model->find_annotation(uri) ;
  if (ann) {
    remove_annotation(uri) ;
    if ((text.size() > 0 || tags.size() > 0) && ann->is_valid()) {
      append_annotation(ann->about(), text, tags, ann) ;
      }
    emit recording_changed(m_recording->uri()) ;
    }
  }

void AnnotationList::delete_annotation(const QString &uri)
/*------------------------------------------------------*/
{
  remove_annotation(uri) ;
  m_recording->delete_resource(rdf::URI(uri.toStdString())) ;
  emit recording_changed(m_recording->uri()) ;
  }
