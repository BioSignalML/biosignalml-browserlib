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

#include "chartplot.h"

#include "annotationdialog.h"

#include <QMenu>
#include <QDialog>
#include <QToolTip>
#include <QMessageBox>
#include <QFileDialog>

#include <algorithm>


using namespace browser ;


void browser::drawtext(QPainter &painter, float x, float y, const QString &text,
/*============================================================================*/
                       bool mapX, bool mapY, int align, float fontSize, int fontWeight)
{
  if (text == "") return ;
  QStringList lines = text.split('\n') ;
  QTransform xfm = painter.transform() ;
  if (mapX || mapY) {
    QPointF pt = xfm.map(QPointF(x, y)) ;  // Assume affine mapping
    if (mapX) x = pt.x() ;
    if (mapY) y = pt.y() ;
    }
  painter.resetTransform() ;
  QFont font = painter.font() ;
  if (fontSize > 0.0 || fontWeight > 0) {
    QFont newfont(font) ;
    if (fontSize > 0.0) newfont.setPointSize(fontSize) ;
    if (fontWeight > 0) newfont.setWeight(fontWeight) ;
    painter.setFont(newfont) ;
    }
  QFontMetrics metrics = painter.fontMetrics() ;
  float th = (metrics.xHeight() + metrics.ascent())/2.0 ;  // Compromise...
  float adjust = (lines.size()-1)*metrics.height() ;       // lineSpacing()
  float tx, ty ;
  if      ((align & alignMiddle) == alignMiddle) ty = y + (th-adjust)/2.0 ;
  else if ((align & alignTop)    == alignTop)    ty = y + th ;
  else                                           ty = y - adjust ;
  for (auto const &t : lines) {
    float tw = metrics.width(t) ;
    if      ((align & alignCentre) == alignCentre) tx = x - tw/2.0 ;
    else if ((align & alignRight) == alignRight)   tx = x - tw ;
    else                                           tx = x ;
    painter.drawText(QPointF(tx, ty), t) ;
    ty += metrics.height() ;                  // lineSpacing()
    }
  painter.setFont(font) ;                     // Reset, in case changed above
  painter.setTransform(xfm) ;
  }


/*
 private:
  
  float m_ymin ; // also Ymin, Ymax
  float m_ymax ;

*/

SignalTrace::SignalTrace(const QString &label, const QString &units,
/*----------------------------------------------------------------*/
                         const bsml::data::TimeSeries::Reference &data, float ymin, float ymax)
: Trace()
{
  m_label = (units == "") ? label : QString("%1\n%2").arg(label, units) ;
  m_selected = false ;
  m_range = NumericRange() ;
  m_ymin = ymin ;
  m_ymax = ymax ;
  m_polygon = QPolygonF() ;
  m_path = QPainterPath() ;
  if (data != nullptr) appendData(data, ymin, ymax) ;
  else                 setYrange() ;
  }

void SignalTrace::setYrange(void)          // sets y values from _y ones...
/*-----------------------------*/
{
  float ymin, ymax ;
  if (m_ymin == m_ymax) {
    if (!isnan(m_ymin)) {
      ymin = m_ymin-std::abs(m_ymin)/2.0 ;
      ymax = m_ymax+std::abs(m_ymax)/2.0 ;
      }
    else {
      ymin = -0.5 ;
      ymax =  0.5 ;
      }
    }
  else {
    ymin = m_ymin ;
    ymax = m_ymax ;
    }
  m_range = NumericRange(ymin, ymax) ;
  m_range_ymin = m_range.start() ;
  m_range_ymax = m_range.end() ;
  }


int SignalTrace::gridheight(void) const
/*-----------------------------------*/
{
  return m_range.major_size() ;
  }


void SignalTrace::appendData(const bsml::data::TimeSeries::Reference &data, float ymin, float ymax)
/*------------------------------------------------------------------------------------------------*/
{
  if (data == nullptr) return ;

  if (isnan(ymin)) ymin = *std::min_element(data->data().begin(), data->data().end()) ;
  if (isnan(ymax)) ymax = *std::max_element(data->data().begin(), data->data().end()) ;
  if (isnan(m_ymin) || m_ymin > ymin) m_ymin = ymin ;
  if (isnan(m_ymax) || m_ymax < ymax) m_ymax = ymax ;
  setYrange() ;           // Will set m_range_ymin/max values from m_ymin/max ones

  QPolygonF poly ;
  // Make TimeSeries an iterator...
  for (auto n = 0 ;  n < data->size() ;  ++n) {
    auto p = data->point(n) ;
    poly.push_back(QPointF(p.time(), p.value())) ;
    }
  //for (auto const &p : data->points()) poly.push_back(QPointF(p.time(), p.value())) ;

  QPainterPath path = QPainterPath() ;
  path.addPolygon(poly) ;
  m_path.connectPath(path) ;
  m_polygon += poly ;
  }


float SignalTrace::yValue(float time) const
/*---------------------------------------*/
{
  int i = index(time) ;
  if (i >= 0) {
    if (i >= (m_polygon.size() - 1)) i = m_polygon.size() - 2 ;
    QPointF p0 = m_polygon.at(i) ;
    QPointF p1 = m_polygon.at(i+1) ;
    return (p0.x() == p1.x()) ? (p0.y() + p1.y())/2.0
                              :  p0.y() + (time - p0.x())*(p1.y() - p0.y())/(p1.x() - p0.x()) ;
    }
  return NAN ;
  }


int SignalTrace::index(float time) const
/*------------------------------------*/
{
  int i = 0 ;
  int j = m_polygon.size() ;
  if (time < m_polygon.at(0).x()
   || time > m_polygon.at(j-1).x()) return -1 ;
  while (i < j) {
    int m = (i + j)/2 ;
    if (m_polygon.at(m).x() <= time) i = m + 1 ;
    else                          j = m ;
    }
  return i - 1 ;
  }


void SignalTrace::drawTrace(QPainter &painter, float start, float end, int labelfreq,
/*---------------------------------------------------------------------------------*/
                            const QVector<float> &markers)
{

  if (m_path.isEmpty()) return ;
  painter.scale(1.0, 1.0/(m_range_ymax - m_range_ymin)) ;
  painter.translate(0.0, -m_range_ymin) ;
  // draw and label y-gridlines.
  int n = 0 ;
  float y = m_range_ymin ;
  while (y <= m_range_ymax) {
    painter.setPen(QPen(gridMinorColour, 0)) ;
    painter.drawLine(QPointF(start, y), QPointF(end, y)) ;
    if (labelfreq > 0
     && m_range_ymin < y && y < m_range_ymax
     && (gridheight()/labelfreq) > 1
     && (n % labelfreq) == 0) {
      painter.drawLine(QPointF(start-0.005*(end-start), y), QPointF(start, y)) ;
      painter.setPen(QPen(textColour, 0)) ;
      drawtext(painter, MARGIN_LEFT-20, y, QString("%1").arg(y), false) ; // Label grid
      }
    y += m_range.major() ;
    if (-1e-10 < y && y < 1e-10) y = 0.0 ; // #####
    n += 1 ;
    }
  painter.setClipping(true) ;
  painter.setPen(QPen(!m_selected ? traceColour : selectedColour, 0)) ;
  // Could find start/end indices and only draw segment
  // rather than rely on clipping...
  painter.drawPath(m_path) ;
  painter.setClipping(false) ;
  if (markers.size() > 0) {
    QTransform xfm = painter.transform() ;
    int n = 0 ;
    for (auto t : markers) {
      painter.setPen(QPen((n == 0) ? markerColour : marker2Colour, 0)) ;
      y = yValue(t) ;
      if (y != NAN) {
        y = m_range.map(y, 1) ;
        QPointF xy = xfm.map(QPointF(t, y)) ;
        drawtext(painter, xy.x()+5, xy.y(), QString("%1").arg(y), false, false, alignLeft) ;
        }
      n += 1 ;
      }
    }
  }


EventTrace::EventTrace(const QString &label, const EventMap &mapping,
/*-----------------------------------------------------------------*/
                       const bsml::data::TimeSeries::Reference &data)
{
  m_label = label ;
  m_selected = false ;
  m_mapping = mapping ;
  m_events = QList<EventInfo>() ;
  m_eventpos = QList<EventPosInfo>() ;
  appendData(data) ;
  }

int EventTrace::gridheight(void) const
/*-----------------------------------*/
{
  return 2 ;    //!< ********* Why 2 ????
  }

QString EventTrace::yPosition(float timepos) const
/*----------------------------------------------*/
{
  timepos = floor(timepos+0.5) + 3 ;                              // "close to"
  int i = 0 ;
  int j = m_eventpos.size() ;
  if (j == 0 || timepos < std::get<0>(m_eventpos[0])) return "" ;
  while (i < j) {
    int m = (i + j)/2 ;
    if (std::get<0>(m_eventpos[m]) <= timepos) i = m + 1 ;
    else                                       j = m ;
    }
  timepos -= 3 ;
  if ((timepos-3) <= std::get<0>(m_eventpos[i-1]) < (timepos+3))  // "close to"
    return std::get<2>(m_eventpos[i-1]) ;
  return "" ;
  }

void EventTrace::appendData(const bsml::data::TimeSeries::Reference &data,
/*----------------------------------------------------------------------*/
                            float ymin, float ymax)      // But unused...
{
  if (data) {
    // Make TimeSeries an iterator...
//    for (auto const pt : data.points()) {  // ??????????
    for (auto n = 0 ;  n < data->size() ;  ++n) {
      auto p = data->point(n) ;
      m_events.append(EventInfo(p.time(), m_mapping(p.value()))) ;
      }
    }
  }

void EventTrace::drawTrace(QPainter &painter, float start, float end,
/*-----------------------------------------------------------------*/
                           int labelfreq, const QVector<float> &markers)
{
  if (m_events.size() == 0) return ;
  painter.setClipping(true) ;
  m_eventpos = QList<EventPosInfo>() ;
  for (auto const &e : m_events) {
    if (e.second != "") {
      float t = e.first ;
      QString text = e.second.split(' ',  QString::SkipEmptyParts).join("\n") ;
      painter.setPen(QPen(!m_selected ? traceColour : selectedColour, 0)) ;
      painter.drawLine(QPointF(t, 0.0), QPointF(t, 1.0)) ;
      painter.setPen(QPen(textColour, 0)) ;
      drawtext(painter, t, 0.5, QString("%1").arg(t)) ;
      QPointF xy = painter.transform().map(QPointF(t, 0.5)) ;
      m_eventpos.append(EventPosInfo(floor(xy.x()+0.5), floor(xy.y()+0.5), text)) ;
      }
    }
  painter.setClipping(false) ;
  }


/*
  A Chart is made up of several Traces stacked vertically
  and all sharing the same X-axis (time axis).

*/
ChartPlot::ChartPlot(QWidget *parent)
/*---------------------------------*/
: QWidget(parent),
  m_id(""), m_plotwidth(0), m_plotheight(0),
  m_timezoom(1.0), m_mousebutton(Qt::NoButton)
{
  setPalette(QPalette(QColor("black"), QColor("white"))) ;
  setMouseTracking(true) ;

  m_traces = QMap<QString, int>() ;
  m_tracelist = TraceList() ;

  m_markers = QList<PosnTime>() ;
  m_marker = -1 ;

  m_selectstart = PosnTime(0, NAN) ;
  m_selectend = PosnTime(0, NAN) ;
  m_selectmove = 0 ;
  m_selecting = false ;
  
  m_annotations = AnnotationDict() ;
  m_annrects = AnnRectList() ;
  m_semantictags = TagDict() ;
  }

void ChartPlot::setId(const QString &id)
/*------------------------------------*/
{
  m_id = id ;
  }

void ChartPlot::setSemanticTags(const TagDict &tags)
/*------------------------------------------------*/
{
  m_semantictags = tags ;
  }

const TagDict &ChartPlot::semanticTags(void) const
/*----------------------------------------------*/
{
  return m_semantictags ;
  }

void ChartPlot::addTrace(const QString &id, bool visible, const std::shared_ptr<Trace> &trace)
/*------------------------------------------------------------------------------------------*/
{
  m_traces[id] = m_tracelist.size() ;
  m_tracelist.append(TraceInfo(id, visible, trace)) ;
  update() ;
  }

void ChartPlot::addSignalTrace(const QString &id, const QString &label, const QString &units,
/*-----------------------------------------------------------------------------------------*/
                               bool visible, 
                               const bsml::data::TimeSeries::Reference &data,
                               float ymin, float ymax)
{
  addTrace(id, visible, std::make_shared<SignalTrace>(label, units, data, ymin, ymax)) ;
  }

void ChartPlot::addEventTrace(const QString &id, const QString &label,
/*------------------------------------------------------------------*/
                              const EventMap &mapping, bool visible,
                              const bsml::data::TimeSeries::Reference &data)
{
  addTrace(id, visible, std::make_shared<EventTrace>(label, mapping, data)) ;
  }

void ChartPlot::appendData(const QString &id, const bsml::data::TimeSeries::Reference &data)
/*----------------------------------------------------------------------------------------*/
{
  int n = m_traces.value(id, -1) ;
  if (n >= 0) {
    std::get<2>(m_tracelist[n])->appendData(data) ;
    update() ;
    }
  }

void ChartPlot::setTraceVisible(const QString &id, bool visible)
/*------------------------------------------------------------*/
{
  int n = m_traces.value(id, -1) ;
  if (n >= 0) {
    std::get<1>(m_tracelist[n]) = visible ;
    update() ;
    }
  }

QStringList ChartPlot::traceOrder(void)
/*-----------------------------------*/
{
  QStringList traces ;
  for (auto const &p : m_tracelist) traces.append(std::get<0>(p)) ;
  return traces ;
  }

void ChartPlot::orderTraces(const QStringList &ids)
/*-----------------------------------------------*/
{
  QList<int> order ;
  TraceList traces ;
  for (auto const &id : ids) {
    int n = m_traces.value(id, -1) ;
    if (n >= 0) {
      order.append(n) ;
      traces.append(m_tracelist[n]) ;
      }
    }

  int i = 0 ;
  std::sort(order.begin(), order.end()) ;
  for (auto const &n : order) {
    m_tracelist[n] = traces[i] ;
    m_traces[std::get<0>(traces[i])] = n ;
    i += 1 ;
    }
  update() ;
  }

void ChartPlot::moveTrace(const QString &from, const QString &to)
/*-------------------------------------------------------------*/
{
  int n = m_traces.value(from, -1) ;
  int m = m_traces.value(to, -1) ;
  if (n >= 0 && m >= 0 && n != m) {
    m_tracelist.move(n, m) ;
    if (n > m) {   // shift up
      for (auto i = m ; i <= n ; ++i)
        m_traces[std::get<0>(m_tracelist[i])] = i ;
      }
    else {         // shift down
      for (auto i = n ; i <= m ; ++i)
        m_traces[std::get<0>(m_tracelist[i])] = i ;
      }
    }
  update() ;
  }

void ChartPlot::plotSelected(const int &row)
/*----------------------------------------*/
{
  int n = 0 ;
  for (auto const &p : m_tracelist) {
    std::get<2>(p)->select((n == row)) ;
    n += 1 ;
    }
  update() ;
  }

void ChartPlot::resetAnnotations(void)
/*----------------------------------*/
{
  m_annotations = AnnotationDict() ;
  m_annrects = AnnRectList() ;
  }

void ChartPlot::addAnnotation(const QString &id, float start, float end, const QString &text,
/*-----------------------------------------------------------------------------------------*/
                              const QStringList &tags, bool edit)
{
  if (isnan(end)) end = start ;
  if (end > m_segmentstart && start < m_segmentend) {
    m_annotations[id] = std::make_tuple(start, end, text, tags, edit) ;
    }
  }

void ChartPlot::deleteAnnotation(const QString &id)
/*-----------------------------------------------*/
{
  m_annotations.remove(id) ;
  update() ;
  }

void ChartPlot::resizeEvent(QResizeEvent *e)
/*----------------------------------------*/
{
  emit chartPosition(pos().x() + MARGIN_LEFT,
                     width() - (MARGIN_LEFT + MARGIN_RIGHT),
                     pos().y() + height()) ;
  }

void ChartPlot::paintEvent(QPaintEvent *e)
/*--------------------------------------*/
{
  draw_window(this) ;
  }

void ChartPlot::draw_window(QPaintDevice *device)
/*---------------------------------------------*/
{
  QPainter qp ;
  qp.begin(device) ;
  qp.setRenderHint(QPainter::Antialiasing) ;

  int w = device->width() ;
  int h = device->height() ;
  m_plotwidth  = w - (MARGIN_LEFT + MARGIN_RIGHT) ;
  m_plotheight = h - (MARGIN_TOP + MARGIN_BOTTOM) ;

  //    if self._id is not None:
  //      drawtext(qp, MARGIN_LEFT+self._plot_width/2, 10, self._id,
  //               fontSize=16, fontWeight=QtGui.QFont.Bold)

  // Set pixel positions of markers and selected region for
  // use in mouse events.
  for (auto &m : m_markers)
    m.first = time_to_pos(m.second) ;
  
  if (!isnan(m_selectstart.second)) {
    m_selectend.first = time_to_pos(m_selectend.second) ;
    m_selectstart.first = time_to_pos(m_selectstart.second) ;
    // Set plotting region as (0, 0) to (1, 1) with origin at bottom left
    }
  qp.translate(MARGIN_LEFT, MARGIN_TOP + m_plotheight) ;
  qp.scale(m_plotwidth, -m_plotheight) ;
  qp.setClipRect(0, 0, 1, 1) ;
  qp.setClipping(false) ;
  qp.setPen(QPen(gridMajorColour, 0)) ;
  qp.drawRect(0, 0, 1, 1) ;

  QTransform labelxfm = qp.transform() ;       // before time transforms

  // Now transform to time co-ordinates
  qp.scale(1.0/(m_windowend - m_windowstart), 1.0) ;
  qp.translate(-m_windowstart, 0.0) ;
  showSelectionRegion(qp) ;   // Highlight selected region
  showAnnotations(qp) ;       // Show annotations
  showSelectionTimes(qp) ;    // Time labels on top of annotation bars
  showTimeMarkers(qp) ;       // Position markers
  draw_time_grid(qp) ;

  // Draw each each visible trace
  int gridheight = 0 ;
  QList<std::shared_ptr<Trace>> traces ;
  for (auto const &p : m_tracelist) {
    if (std::get<1>(p)) {
      auto trace = std::get<2>(p) ;
      gridheight += trace->gridheight() ;
      traces.append(trace) ;
      }
    }

  int labelfreq ;
  if (m_plotheight == 0) labelfreq = 0 ;
  else                   labelfreq = (int)(10.0/(float)m_plotheight/(float)(gridheight + 1)) + 1 ;

  float traceposition = gridheight ;
  for (auto const &tp : traces) {
    qp.save() ;
    float traceheight = tp->gridheight() ;
    qp.scale(1.0, traceheight/(float)gridheight) ;
    traceposition -= traceheight ;
    qp.translate(0.0, traceposition/traceheight) ;
    QVector<float> markers(m_markers.size()) ;
    for (auto const &m : m_markers) markers.append(m.second) ;
    tp->drawTrace(qp, m_windowstart, m_windowend, labelfreq, markers) ;
    qp.restore() ;
    }

  // Event labels have now been assigned (by drawTrace() above) so now show them
  qp.setTransform(labelxfm) ;
  traceposition = gridheight ;
  for (auto const &tp : traces) {
    qp.save() ;
    float traceheight = tp->gridheight() ;
    qp.scale(1.0, traceheight/(float)gridheight) ;
    traceposition -= traceheight ;
    qp.translate(0.0, traceposition/traceheight) ;
    qp.setPen(QPen(textColour, 0)) ;
    drawtext(qp, (MARGIN_LEFT-40)/2, 0.5, tp->label(), false) ;
    int n = 0 ;
    for (auto const &m : m_markers) {
      QString ytext = tp->yPosition(m.first) ;
      if (ytext != "") {                 // Write event descriptions on RHS
        qp.setPen(QPen((n == 0) ? markerColour : marker2Colour, 0)) ;
        drawtext(qp, MARGIN_LEFT+m_plotwidth+25, 0.50, ytext, false) ;
        }
      n += 1 ;
      }
    qp.restore() ;
    }

  qp.end() ;                     // Done all drawing
  }

void ChartPlot::showSelectionRegion(QPainter &painter)
/*--------------------------------------------------*/
{
  if (m_selectstart.second != m_selectend.second) {
    float duration = (m_selectend.second - m_selectstart.second) ;
    painter.setClipping(true) ;
    painter.fillRect(QRectF(m_selectstart.second, 0.0, duration, 1.0), selectionColour) ;
    painter.setPen(QPen(selectEdgeColour, 0)) ;
    painter.drawLine(QPointF(m_selectstart.second, 0), QPointF(m_selectstart.second, 1.0)) ;
    painter.drawLine(QPointF(m_selectend.second,   0), QPointF(m_selectend.second,   1.0)) ;
    painter.setClipping(false) ;
    }
  }

void ChartPlot::showSelectionTimes(QPainter &painter)
/*-------------------------------------------------*/
{
  if (m_selectstart.second != m_selectend.second) {
    float duration = (m_selectend.second - m_selectstart.second) ;
    float ypos = MARGIN_TOP - 8.0 ;          //!< ***** Why 8 ??? CONSTANT to header
    painter.setPen(QPen(selectTimeColour, 0)) ;
    drawtext(painter, m_selectstart.second, ypos, QString("%1").arg(m_selectstart.second), true, false) ;
    drawtext(painter, m_selectend.second,   ypos, QString("%1").arg(m_selectend.second),   true, false) ;
    painter.setPen(QPen(selectLenColour, 0)) ;
    float middle = (m_selectend.second + m_selectstart.second)/2.0 ;
    if (duration < 0.0) duration = -duration ;
    drawtext(painter, middle, ypos, QString("%1").arg(duration), true, false) ;
    }
  }

void ChartPlot::draw_time_grid(QPainter &painter)
/*---------------------------------------------*/
{
  QTransform xfm = painter.transform() ;
  painter.resetTransform() ;
  int ypos = MARGIN_TOP + m_plotheight ;
  painter.setPen(QPen(gridMinorColour, 0)) ;
  float t = m_timerange.start() ;
  while (t <= m_windowend) {
    if (m_windowstart <= t <= m_windowend)
      painter.drawLine(QPoint(time_to_pos(t), MARGIN_TOP),
                       QPoint(time_to_pos(t), ypos)) ;
    t += m_timerange.minor() ;
    }
  t = m_timerange.start() ;
  while (t <= m_windowend) {
    if (m_windowstart <= t && t <= m_windowend) {
      painter.setPen(QPen(gridMajorColour, 0)) ;
      painter.drawLine(QPoint(time_to_pos(t), MARGIN_TOP),
                       QPoint(time_to_pos(t), ypos+5)) ;
      painter.setPen(QPen(textColour, 0)) ;
      drawtext(painter, time_to_pos(t), ypos+18, QString("%1").arg(t), false, false) ;
      }
    t += m_timerange.major() ;
    }
  drawtext(painter, MARGIN_LEFT+m_plotwidth+40, ypos+18, "Time\n(secs)", false, false) ;
  painter.setTransform(xfm) ;
  }

void ChartPlot::setTimeGrid(float start, float end)
/*-----------------------------------------------*/
{
  m_timerange = NumericRange(start, end) ;
  m_windowstart = start ;
  m_windowend = end ;
  }

void ChartPlot::showTimeMarkers(QPainter &painter)
/*----------------------------------------------*/
{
  QTransform xfm = painter.transform() ;
  painter.resetTransform() ;

  int n = 0 ;
  PosnTime last(0, 0.0) ;
  for (auto const &m : m_markers) {
    float ypos = MARGIN_TOP - 20 ;
    painter.setPen(QPen((n == 0) ? markerColour : marker2Colour, 0)) ;
    painter.drawLine(QPoint(m.first, ypos + 6),
                     QPoint(m.first, MARGIN_TOP+m_plotheight+10)) ;
    drawtext(painter, m.first, ypos, QString("%1").arg(m_timerange.map(m.second)), false, false) ;
    if (n > 0 && m.second != last.second) {
      painter.setPen(QPen(textColour, 0)) ;
      float width = m_timerange.map(last.second) - m_timerange.map(m.second) ;
      if (width < 0) width = -width ;
      drawtext(painter, (last.first+m.first)/2.0, ypos, QString("%1").arg(width), false, false) ;
      }
    last = m ;
    n += 1 ;
    }
  painter.setTransform(xfm) ;
  }

void ChartPlot::showAnnotations(QPainter &painter)
/*----------------------------------------------*/
{
  QTransform xfm = painter.transform() ;
  painter.resetTransform() ;
  int right_side = MARGIN_LEFT + m_plotwidth ;
  int line_space = ANN_LINE_WIDTH + ANN_LINE_GAP ;
  // Sort (into time order), start from top, and not
  // step down if prev. end <= new start
  // Save bar rectangle for finding tool tip...
  m_annrects = AnnRectList() ;           // list of (rect, id) pairs
  QList<QPair<float, int>> endtimes ;    // [endtime, colour] pair for each row
  
  int nextcolour = 0 ;
  QHash<QString, int> colourdict ;  // key by text, to use the same colour for the same text

  typedef QPair<AnnInfo, QString> AnnSort ;
  QList<AnnSort> sorted ;
  for (auto const &id : m_annotations.keys())
    sorted.append(QPair<AnnInfo, QString>(m_annotations[id], id)) ;
  auto compare = [] (const AnnSort &s1, const AnnSort &s2)
    { return std::get<0>(s1.first) < std::get<0>(s2.first)
         || (std::get<0>(s1.first) == std::get<0>(s2.first)
          && std::get<1>(s1.first) < std::get<1>(s2.first)) ;
    } ;
  std::sort(sorted.begin(), sorted.end(), compare) ;  // In start time order...

  for (auto const &annid : sorted) {
    AnnInfo ann = annid.first ;
    QString id = annid.second ;
    QList<int> colours ;  colours << -1 << -1 << -1 ;   // Left, above, below
    int row = -1 ;

    int n = 0 ;
    for (auto &e : endtimes) {
      n += 1 ;
      if (std::get<0>(ann) > e.first) {      // Start time after last end on this row?
        row = n ;
        e.first = std::get<1>(ann) ;        // Save end time
        colours[0] = e.second ;
        if ((n + 1) < endtimes.size()) {
          colours[2] = endtimes[n+1].second ;
          }
        break ;
        }
      colours[1] = e.second ;
      n += 1 ;
      }
    if (row == -1) {
      row = endtimes.size() ;
      endtimes.append(QPair<float, int>(std::get<1>(ann), -1)) ;
      }
    int ann_top = ANN_START + row*line_space ;
    QString text = annotation_display_text(ann) ;
    int thiscolour = colourdict.value(text, -1) ;
    if (thiscolour == -1) {
      int l = ANN_COLOURS.size() ;
      thiscolour = nextcolour ;
      while (colours.contains(thiscolour)) {                 // Must terminate since
        thiscolour = (thiscolour + 1) % ANN_COLOURS.size() ; // ANN_COLOURS.size() > colours.size()
        }
      nextcolour = (nextcolour + 1) % ANN_COLOURS.size() ;
      colourdict[text] = thiscolour ;
      }
    endtimes[row].second = thiscolour ;  // Save colour index
    QColor colour = ANN_COLOURS[thiscolour] ;
    QPen pen(colour, 0) ;
    // pen.setCapStyle(QtCore.Qt.FlatCap)
    // pen.setWidth(1)
    painter.setPen(pen) ;
    int xstart = time_to_pos(std::get<0>(ann)) ;
    int xend = time_to_pos(std::get<1>(ann)) ;
    if (MARGIN_LEFT < xstart < right_side)
      painter.drawLine(QPoint(xstart, ann_top),
                       QPoint(xstart, MARGIN_TOP+m_plotheight)) ;
    if (MARGIN_LEFT < xend < right_side)
      painter.drawLine(QPoint(xend, ann_top),
                       QPoint(xend, MARGIN_TOP+m_plotheight)) ;
    if (xstart < right_side && MARGIN_LEFT < xend) {
      int left = std::max(MARGIN_LEFT, xstart) ;
      int right = std::min(xend, right_side) ;
      int width = right - left ;
      if (width <= 1) {                     //#  Instants
        left -= 2 ;
        width = 4 ;
        }
      QRect rect(left, ann_top, // - ANN_LINE_WIDTH/2,
                 width, ANN_LINE_WIDTH) ;
      // pen.setWidth(ANN_LINE_WIDTH)
      // painter.setPen(pen)
      painter.fillRect(rect, colour) ;
      rect = QRect(left, ann_top - ANN_LINE_WIDTH,    // Untransformed
                   width, ANN_LINE_WIDTH) ;           // for contains(pos)
      m_annrects.append(QPair<QRect, QString>(rect, id)) ;
      }
    }
  painter.setTransform(xfm) ;
  }

float ChartPlot::pos_to_time(int pos)
/*---------------------------------*/
{
  float time = m_windowstart + m_windowduration*(pos - MARGIN_LEFT)/(float)m_plotwidth ;
  if (time < m_windowstart) time = m_windowstart ;
  if (time > m_windowend) time = m_windowend ;
  return m_timerange.map(time) ;
  }

int ChartPlot::time_to_pos(float time)
/*----------------------------------*/
{
  return MARGIN_LEFT + (time - m_windowstart)*m_plotwidth/m_windowduration ;
  }

void ChartPlot::setTimeRange(float start, float duration)
/*-----------------------------------------------------*/
{
  m_segmentstart = m_windowstart = start ;
  m_segmentend = m_windowend = start + duration ;
  m_duration = duration ;
  setTimeZoom(m_timezoom) ;    // Keep existing zoom
  m_markers = QList<PosnTime>{PosnTime(0, m_windowstart), PosnTime(0, m_windowstart)} ; // Two markers
  }

void ChartPlot::setTimeZoom(float scale)
/*------------------------------------*/
{
  m_timezoom = scale ;
  m_windowduration = m_duration/scale ;

  float newstart = (m_windowstart + m_windowend - m_windowduration)/2.0 ;
  float newend = newstart + m_windowduration ;
  if (newstart < m_segmentstart) {
    newstart = m_segmentstart ;
    newend = newstart + m_windowduration ;
    }
  else if (newend > m_segmentend) {
    newend = m_segmentend ;
    newstart = newend - m_windowduration ;
    // Now update slider's position to reflect _start _position _end
    }
  setTimeGrid(newstart, newend) ;
  //for m in self._markers: m[0] = self._time_to_pos(m[1])
  update() ;
  }

void ChartPlot::setTimeScroll(QScrollBar &scrollbar)
/*------------------------------------------------*/
{
  scrollbar.setMinimum(0) ;
  int scrollwidth = 1000 ;
  scrollbar.setPageStep(scrollwidth/m_timezoom) ;
  scrollbar.setMaximum(scrollwidth - scrollbar.pageStep()) ;
  scrollbar.setValue(scrollwidth*(m_windowstart - m_segmentstart)/m_duration) ;
  }

void ChartPlot::moveTimeScroll(QScrollBar &scrollbar)
/*-------------------------------------------------*/
{
  float start = (m_segmentstart
              + scrollbar.value()*m_duration/(scrollbar.maximum()+scrollbar.pageStep())) ;
  setTimeGrid(start, start + m_windowduration) ;
  //    for m in self._markers:                       ## But markers need to scroll...
  //      if m[1] < self._start: m[1] = self._start
  //      if m[1] > self._end: m[1] = self._end
  update() ;
  }

void ChartPlot::setMarker(float time)
/*---------------------------------*/
{
  m_markers[0] = PosnTime(time_to_pos(time), m_timerange.map(time)) ;
  }

void ChartPlot::mousePressEvent(QMouseEvent *event)
/*-----------------------------------------------*/
{
  m_mousebutton = event->button() ;
  if (m_mousebutton != Qt::LeftButton) return ;
  
  QPoint pos = event->pos() ;
  int xpos = pos.x() ;
  float xtime = pos_to_time(xpos) ;
  // check right click etc...

  PosnTime marker(-1, 0.0) ;

  if (pos.y() <= MARGIN_TOP) {

    QList<QPair<int, int>> mpos ;
    int n = 0 ;
    for (auto const &m : m_markers)
      mpos.append(QPair<int, int>(m.first, n)) ;
    std::sort(mpos.begin(), mpos.end()) ;

    if (xpos <= mpos[0].first) {
      m_marker = mpos[0].second ;
      }
    else if (xpos >= mpos[-1].first) {
      m_marker = mpos[-1].second ;
      }
    else {
      n = 0 ;
      for (auto const &m : mpos) {
        if (n >= (mpos.size()-1)) break ;
        float mid = (m.first + mpos[n+1].first)/2.0 ;
        if (xpos <= mid) {
          m_marker = m.second ;
          break ;
          }
        else if (xpos <= mpos[n+1].first) {
          m_marker = mpos[n+1].second ;
          break ; ;
          }
        n += 1 ;
        }
      }
    marker = m_markers[m_marker] ;
    }
  else {
    int n = 0 ;
    for (auto const &m : m_markers) {
      if ((xpos-2) <= m.first <= (xpos+2)) {
        m_marker = n ;
        marker = m ;
        break ;
        }
      n += 1 ;
      }
    }

  if (marker.first >= 0) {
    marker = PosnTime(xpos, xtime) ;
    }
  else if (MARGIN_TOP < pos.y() <= (MARGIN_TOP + m_plotheight)) {
    //# Need to be able to clear selection (click inside??)
    //# and start selecting another region (drag outside of region ??)
    if (isnan(m_selectstart.second)) {
      m_selectstart = PosnTime(xpos, xtime) ;
      m_selectend = m_selectstart ;
      }
    else if ((xpos-2) <= m_selectstart.first && m_selectstart.first <= (xpos+2)) {
      auto end = m_selectend ;                       // Start edge move
      m_selectend = m_selectstart ;
      m_selectstart = end ;
      }
    else if (((m_selectstart.first+2) < xpos && xpos < (m_selectend.first-2))
          || ((m_selectend.first+2)   < xpos && xpos < (m_selectstart.first-2))) {
      m_selectmove = xpos ;
      }
    else if (!((xpos-2) <= m_selectend.first && m_selectend.first <= (xpos+2))) {
      m_selectstart = PosnTime(xpos, xtime) ;
      m_selectend = m_selectstart ;
      }
    m_selecting = true ;
    }
  update() ;
  }

QString ChartPlot::annotation_display_text(const AnnInfo &ann)
/*----------------------------------------------------------*/
{
  QStringList text ;
  if (std::get<2>(ann) != "")
    text.append("<p>" + std::get<2>(ann) + "</p>") ;
  if (std::get<3>(ann).size() > 0) {
    QStringList tags ;
    for (auto const &t : std::get<3>(ann))
      tags.append(m_semantictags.value(t, t)) ;
    text.append("<p>Tags: " + tags.join(", ") + "</p>") ;
    }
  return text.join("") ;
  }

void ChartPlot::mouseMoveEvent(QMouseEvent *event)
/*----------------------------------------------*/
{
  int xpos = event->pos().x() ;
  int ypos = event->pos().y() ;
  float xtime = pos_to_time(xpos) ;
  bool tooltip = false ;
  if (m_mousebutton == Qt::NoButton) {
    for (auto const &a : m_annrects) {
      if (a.first.contains(xpos, ypos)) {
        QFont font = QToolTip::font() ;
        font.setPointSize(16) ;          // Magic font size...
        QToolTip::setFont(font) ;
        QToolTip::showText(event->globalPos(), annotation_display_text(m_annotations[a.second])) ;
        tooltip = true ;
        break ;
        }
      }
    }
  else if (m_marker >= 0) {
    m_markers[m_marker] = PosnTime(xpos, xtime) ;
    update() ;
    }
  else if (m_selecting) {
    if (m_selectmove == 0) {
      m_selectend = PosnTime(xpos, xtime) ;
      }
    else {
      int delta = xpos - m_selectmove ;
      m_selectmove = xpos ;
      m_selectend.first += delta ;
      m_selectend.second = m_timerange.map(m_selectend.first) ;
      m_selectstart.first += delta ;
      m_selectstart.second = m_timerange.map(m_selectstart.first) ;
      }
    update() ;
    }
  if (!tooltip) QToolTip::showText(event->globalPos(), "") ;
  }

void ChartPlot::mouseReleaseEvent(QMouseEvent *event)
/*-------------------------------------------------*/
{
  if (m_mousebutton == Qt::LeftButton) {
    m_marker = -1 ;
    if (m_selecting) {
      if (m_selectstart.first > m_selectend.first) { // Moved start edge
        auto end = m_selectend ;
        m_selectend = m_selectstart ;
        m_selectstart = end ;
        }
      m_selecting = false ;
      m_selectmove = 0 ;
      }
    }
  m_mousebutton = Qt::NoButton ;
  }

void ChartPlot::contextMenuEvent(QContextMenuEvent *event)
/*------------------------------------------------------*/
{
  QPoint pos = event->pos() ;
  m_mousebutton = Qt::NoButton ;
  for (auto const &a : m_annrects) {
    if (a.first.contains(pos)) {
      QString ann_id = a.second ;
      AnnInfo ann = m_annotations[ann_id] ;
      if (std::get<4>(ann)) {  // editable
        QMenu menu ;
        menu.addAction("Edit") ;
        menu.addAction("Delete") ;
        QAction *item = menu.exec(QWidget::mapToGlobal(pos)) ;
        if (item) {
          if (item->text() == "Edit") {
            AnnotationDialog dialog(this, m_id, std::get<0>(ann), std::get<1>(ann), std::get<2>(ann), std::get<3>(ann)) ;
            if (dialog.exec() == QDialog::Accepted) {
              QString text = dialog.get_annotation() ;
              QStringList tags = dialog.get_tags() ;
              if ((text != "" && text != std::get<2>(ann).trimmed())
                || tags != std::get<3>(ann)) emit annotationModified(ann_id, text, tags) ;
              }
            }
          else if (item->text() == "Delete") {
            QMessageBox confirm(QMessageBox::Question, "Delete Annotation",
              "Delete Annotation", QMessageBox::Cancel | QMessageBox::Ok) ;
            confirm.setInformativeText("Do you want to delete the annotation?") ;
            confirm.setDefaultButton(QMessageBox::Cancel) ;
            if (confirm.exec() == QMessageBox::Ok) emit annotationDeleted(ann_id) ;
            }
          }
        }
      return ;
      }
    }
  if (MARGIN_TOP  < pos.y() && pos.y() <= (MARGIN_TOP + m_plotheight)
   && MARGIN_LEFT < pos.x() && pos.x() <= (MARGIN_LEFT + m_plotwidth)) {
    QMenu menu ;
    if (m_selectstart != m_selectend
     && m_selectstart.first < pos.x() && pos.x() < m_selectend.first) {
      menu.addAction("Zoom") ;
      menu.addAction("Annotate") ;
      menu.addAction("Export") ;
      QAction *item = menu.exec(QWidget::mapToGlobal(pos)) ;
      if (item) {
        bool clearselection = false ;
        if (item->text() == "Zoom") {
          float scale = m_duration/(m_selectend.second - m_selectstart.second) ;
          m_windowstart = m_selectstart.second ;
          m_windowend   = m_selectend.second ;
          // emit zoomChart(scale) ;      // Results in setTimeZoom() being called
          setTimeZoom(scale) ;            // TEMP ???
          clearselection = true ;
          }
        else if (item->text() == "Annotate") {
          AnnotationDialog dialog(this, m_id, m_selectstart.second, m_selectend.second) ;
          if (dialog.exec() == QDialog::Accepted) {
            QString text = dialog.get_annotation() ;
            QStringList tags = dialog.get_tags() ;
            if (text != "" || tags.size() > 0) {
              emit annotationAdded(m_selectstart.second, m_selectend.second, text, tags) ;
              clearselection = true ;
              }
            }
          }
        else if (item->text() == "Export") {
          QString filename = QFileDialog::getSaveFileName(this, "Export region", "", "*.bsml") ;
          if (filename != "") {
            emit exportRecording(filename, m_selectstart.second, m_selectend.second) ;
            clearselection = true ;
            }
          }
        if (clearselection) m_selectend = m_selectstart ;
        update() ;
        }
      }
    else {
      if (m_timezoom > 1.0) menu.addAction("Reset zoom") ;   // Have but disabled...
      menu.addAction("Save as PNG") ;
      QAction *item = menu.exec(QWidget::mapToGlobal(pos)) ;
      if (item) {
        if (item->text() == "Reset zoom") {
          // emit zoomChart(1.0) ;       // Results in setTimeZoom() being called
          m_timezoom = 1.0 ;             // ????
          setTimeRange(0.0, m_duration) ;       //# TEMP ???
          }
        else if (item->text() == "Save as PNG") {
          QString filename = QFileDialog::getSaveFileName(this, "Save chart", "", "*.png") ;
          if (filename != "") {
            QImage output(width(), height(), QImage::Format_ARGB32_Premultiplied) ;
            draw_window(&output) ;
            output.save(filename, "PNG") ;
            }
          }
        }
      }
    }
  }
