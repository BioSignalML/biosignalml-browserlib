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

#ifndef BROWSER_CHARTPLOT_H
#define BROWSER_CHARTPLOT_H

#include "nrange.h"

#include <biosignalml/data/data.h>

#include <QColor>
#include <QPainter>
#include <QPolygonF>
#include <QPainterPath>
#include <QVector>
#include <QScrollBar>
#include <QMouseEvent>

#include <cmath>


namespace browser {

  // Margins of plotting region within chart, in pixels
  static const int MARGIN_LEFT   =  120 ;
  static const int MARGIN_RIGHT  =   80 ;
  static const int MARGIN_TOP    =  100 ;
  static const int MARGIN_BOTTOM =   40 ;

  static const QColor traceColour("green") ;
  static const QColor selectedColour("red") ; // When signal is selected in controller
  static const QColor textColour("darkBlue") ;
  static const QColor markerColour("blue") ;
  static const QColor marker2Colour(0xCC, 0x55, 0x00) ;  // 'burnt orange'
  static const QColor gridMinorColour(128, 128, 255, 63) ;
  static const QColor gridMajorColour(0,     0, 128, 63) ;
  static const QColor selectionColour(220, 255, 255) ;   // Of selected region
  static const QColor selectEdgeColour("cyan") ;
  static const QColor selectTimeColour("black") ;
  static const QColor selectLenColour("darkRed") ;

  static const int ANN_START =       20 ;          // Pixels from top to first bar
  static const int ANN_LINE_WIDTH =   8 ;
  static const int ANN_LINE_GAP =     2 ;

  static QVector<QColor> ANN_COLOURS = {
    QColor("red"),      QColor("blue"),     QColor("magenta"),
    QColor("darkRed"),  QColor("darkBlue"), QColor("cyan")
    } ;

  static const int alignLeft    =  0x01 ;
  static const int alignRight   =  0x02 ;
  static const int alignCentre  =  0x03 ;
  static const int alignTop     =  0x04 ;
  static const int alignBottom  =  0x08 ;
  static const int alignMiddle  =  0x0C ;
  static const int alignCentred =  0x0F ;


  class Trace ;

  typedef std::function<QString (float)> EventMap ;
  typedef QPair<float, QString> EventInfo ;
  typedef std::tuple<float, float, QString> EventPosInfo ;

  typedef QHash<QString, QString> TagDict ;  // { uri: label }

  typedef std::tuple<QString, bool, std::shared_ptr<Trace>> TraceInfo ;
  typedef QList<TraceInfo> TraceList ;

  typedef QPair<int, float> PosnTime ;

  typedef std::tuple<float, float, QString, QStringList, bool> AnnInfo ;
  typedef QMap<QString, AnnInfo> AnnotationDict ;

  typedef QList<QPair<QRect, QString>> AnnRectList ; // List of tuple(rect, id)


  static void drawtext(QPainter &painter, float x, float y, const QString &text,
                       bool mapX=true, bool mapY=true, int align=alignCentred,
                       float fontSize=0.0, int fontWeight=0) ;


  class Trace
  /*=======*/
  {
   public:
    Trace() = default ;
    virtual ~Trace() = default ;

    inline const QString &label(void) const { return m_label ; }
    inline void select(bool selected) { m_selected = selected ; }

    /* The height, in pixels, of a trace on its grid. */
    virtual int gridheight(void) const = 0 ;

    virtual QString yPosition(float timepos) const { return "" ; }

    /** Add data to a trace. */
    virtual void appendData(const bsml::data::TimeSeries::Reference &data, float ymin=NAN, float ymax=NAN) = 0 ;

    /** Draw the trace.
     *
     * :param painter: The QPainter to use for drawing.
     * :param start: The leftmost position on the X-axis.
     * :param end: The rightmost position on the X-axis.
     *
     * The painter has been scaled so that (0.0, 1.0) is the
     * vertical plotting height.
     */
    virtual void drawTrace(QPainter &painter, float start, float end,
                           int labelfreq, const QVector<float> &markers) = 0 ;

   protected:
    QString m_label ;
    bool m_selected ;
    } ;


  /** A single data trace. **/
  class SignalTrace : public Trace
  /*============================*/
  {
   public:
    SignalTrace(const QString &label, const QString &units,
                const bsml::data::TimeSeries::Reference &data=nullptr,
                float ymin=NAN, float ymax=NAN) ;

    int gridheight(void) const ;
    void appendData(const bsml::data::TimeSeries::Reference &data, float ymin=NAN, float ymax=NAN) ;
    void drawTrace(QPainter &painter, float start, float end, int labelfreq, const QVector<float> &markers) ;

   private:
    /** Find the y-value corresponding to a time. */
    float yValue(float time) const ;
    int index(float time) const ;
    void setYrange(void) ;

    float m_ymin ;
    float m_ymax ;
    NumericRange m_range ;
    float m_range_ymin ;
    float m_range_ymax ;
    QPainterPath m_path ;
    QPolygonF m_polygon ;
    } ;


  /** A single event trace. */
  class EventTrace : public Trace
  /*===========================*/
  {
   public:
    EventTrace(const QString &label,
      const EventMap &mapping=[](float x) { return QString("%1").arg(x) ; },
      const bsml::data::TimeSeries::Reference &data=nullptr) ;

    int gridheight(void) const ;
    QString yPosition(float timepos) const ;
    void appendData(const bsml::data::TimeSeries::Reference &data, float ymin=NAN, float ymax=NAN) ;
    void drawTrace(QPainter &painter, float start, float end, int labelfreq, const QVector<float> &markers) ;

   private:
    EventMap m_mapping ;             //!< Event code --> text
    QList<EventInfo> m_events ;
    QList<EventPosInfo> m_eventpos ;
    } ;


  /**
   * A Chart is made up of several Traces stacked vertically
   * and all sharing the same X-axis (time axis).
   */
  class ChartPlot : public QWidget
  /*============================*/
  {
   public:
    ChartPlot(QWidget *parent=nullptr) ;

    void setId(const QString &id) ;
    void setSemanticTags(const TagDict &tags) ;
    const TagDict &semanticTags(void) const ;
    void setTimeScroll(QScrollBar &scrollbar) ;
    void moveTimeScroll(QScrollBar &scrollbar) ;

    void resizeEvent(QResizeEvent *e) ;
    void paintEvent(QPaintEvent *e) ;
    void mousePressEvent(QMouseEvent *event) ;
    void mouseMoveEvent(QMouseEvent *event) ;
    void mouseReleaseEvent(QMouseEvent *event) ;
    void contextMenuEvent(QContextMenuEvent *event) ;

   public slots:
    void addSignalTrace(const QString &id, const QString &label, const QString &units,
                        bool visible=true,
                        const bsml::data::TimeSeries::Reference &data=nullptr,
                        float ymin=NAN, float ymax=NAN) ;
    void addEventTrace(const QString &id, const QString &label,
                       const EventMap &mapping=[](float x) { return QString("%1").arg(x) ; },
                       bool visible=true,
                       const bsml::data::TimeSeries::Reference &data=nullptr) ;
    void appendData(const QString &id, const bsml::data::TimeSeries::Reference &data) ;
    void setTraceVisible(const QString &id, bool visible=true) ;

    /** Get list of trace ids in display order. */
    QStringList traceOrder(void) ;

    /**
     * Reorder display to match id list.
     *
     * The display position of plots is changed to match the order
     * of ids in the list. Plots with ids not in the list are not
     * moved. Unknown ids are ignored.
     */
    void orderTraces(const QStringList &ids) ;

    /** Move a trace, shifting others up or down. */
    void moveTrace(const QString &from, const QString &to) ;

    void plotSelected(const int &row) ;
    void resetAnnotations(void) ;
    void addAnnotation(const QString &id, float start, float end, const QString &text,
                       const QStringList &tags, bool edit=false) ;
    void deleteAnnotation(const QString &id) ;
    void setTimeRange(float start, float duration) ;
    void setTimeZoom(float scale) ;
    void setMarker(float time) ;

   signals:
    void chartPosition(int offset, int width, int bottom) ;
    void updateTimeScroll(bool visible) ;
    void annotationAdded(float start, float end, const QString &text, const QStringList &tags) ;
    void annotationModified(const QString &id, const QString &text, const QStringList &tags) ;
    void annotationDeleted(const QString &id) ;
    void exportRecording(const QString &filename, float start, float end) ;
    void zoomChart(float scale) ;

   private:
    void draw_window(QPaintDevice *device) ;

    void addTrace(const QString &id, bool visible, const std::shared_ptr<Trace> &trace) ;
    void draw_trace_labels(QPainter &painter) ;
    void showSelectionRegion(QPainter &painter) ;
    void showSelectionTimes(QPainter &painter) ;
    void draw_time_grid(QPainter &painter) ;
    void setTimeGrid(float start, float end) ;
    void showTimeMarkers(QPainter &painter) ;
    void showAnnotations(QPainter &painter) ;
    float pos_to_time(int pos) ;
    int time_to_pos(float time) ;
    QString annotation_display_text(const AnnInfo &ann) ;

    QString m_id ;                 //!< Identifier (URI) of segment
    float m_segmentstart ;         //!< Start time of segment
    float m_segmentend  ;          //!< End time of segment
    float m_duration ;             //!< Duration of segment

    float m_timezoom ;             //!< Scale of window into segment
    float m_windowduration ;       //!< Duration of window
    float m_windowstart ;          //!< Start time of window
    float m_windowend   ;          //!< End time of window

    int m_plotwidth ;              //!< Width of window in pixels
    int m_plotheight ;             //!< Height of window in pixels

    QMap<QString, int> m_traces ;  //!< id --> index in tracelist
    TraceList m_tracelist ;        //!< [id, visible, plot] triples as a list

    NumericRange m_timerange ;

    QList<PosnTime> m_markers ;    //!< List of [xpos, time] pairs
    int m_marker ;                 //!< Index of marker being dragged

    PosnTime m_selectstart ;
    PosnTime m_selectend ;
    int m_selectmove ;
    bool m_selecting ;

    Qt::MouseButton m_mousebutton ;

    TagDict m_semantictags ;       //!< uri --> label

    AnnotationDict m_annotations ; //!< id --> to tuple(start, end, text, tags, editable)
    AnnRectList m_annrects ;
    } ;

  } ;

#endif
