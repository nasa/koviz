#include "bookview_curves.h"

CurvesView::CurvesView(QWidget *parent) :
    BookIdxView(parent),
    _pixmap(0),
    _isMeasure(false),
    _isLastPoint(false),
    _bw_frame(0),
    _bw_label(0),
    _bw_slider(0),
    _sg_frame(0),
    _sg_window(0),
    _sg_slider(0),
    _integ_frame(0)
{
    setFocusPolicy(Qt::StrongFocus);
    setFrameShape(QFrame::NoFrame);

    // Set mouse tracking to receive mouse move events when button not pressed
    setMouseTracking(true);
}

CurvesView::~CurvesView()
{
    delete _pixmap;

    foreach ( FFTCurveCache* cache,  _fftCache.curveCaches ) {
        delete cache->curveModel();
        delete cache;
    }
}

void CurvesView::setCurrentCurveRunID(int runID)
{
    if ( runID < 0 ) {
        // Set current curves view index to nothing
        setCurrentIndex(QModelIndex());
        return;
    }

    QModelIndex cidx = currentIndex();
    QString tag = _bookModel()->data(cidx).toString();

    if ( cidx.isValid() && tag == "Curve" ) { //Choose curve based on curr curve

        // See if current curve's runid is runID
        int curveRunID = _bookModel()->getDataInt(cidx,"CurveRunID","Curve");
        if ( curveRunID == runID ) {
            // Nothing to do since current curve runID is already runID
            return;
        }

        // Search for curve with runID and same yName as current curve
        QString currYName = _bookModel()->getDataString(cidx,
                                                        "CurveYName","Curve");
        QModelIndex plotIdx = rootIndex();
        QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
        int rc = model()->rowCount(curvesIdx);
        for (int i = 0 ; i < rc; ++i ) {
            QModelIndex curveIdx = model()->index(i,0,curvesIdx);
            if ( model()->data(curveIdx).toString() == "Curve" ) {
                QString yName = _bookModel()->getDataString(curveIdx,
                                                         "CurveYName", "Curve");
                int curveRunID = _bookModel()->getDataInt(curveIdx,
                                                          "CurveRunID","Curve");
                if ( curveRunID == runID && yName == currYName ) {
                    // Found curve with runId and same curve name as current
                    setCurrentIndex(curveIdx);
                    return;
                }
            }
        }
    }

    // Search for first curve that matches runID
    QModelIndex plotIdx = rootIndex();
    QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
    int rc = model()->rowCount(curvesIdx);
    for (int i = 0 ; i < rc; ++i ) {
        QModelIndex curveIdx = model()->index(i,0,curvesIdx);
        if ( model()->data(curveIdx).toString() == "Curve" ) {
            int curveRunID = _bookModel()->getDataInt(curveIdx,
                                                      "CurveRunID","Curve");
            if ( curveRunID == runID ) {
                // Found curve with same runID, make it current
                setCurrentIndex(curveIdx);
                return;
            }
        }
    }
}

void CurvesView::markTime(const QString &label, double time, int timeIdx)
{
    QString tag = _bookModel()->data(currentIndex()).toString();

    QString pres = _bookModel()->getDataString(rootIndex(),
                                               "PlotPresentation","Plot");

    QModelIndex pIdx; // Block below sets pidx to parent of Markers
    if ( currentIndex().isValid() && (tag == "Curve" || tag == "Plot") ) {
        pIdx = currentIndex();
    } else if ( pres == "error" ) {
        pIdx = rootIndex();
    } else {
        QModelIndex curvesIdx = _bookModel()->getIndex(rootIndex(),
                                                       "Curves","Plot");
        int nCurves = model()->rowCount(curvesIdx);
        if ( nCurves > 0 ) {
            pIdx = model()->index(0,0,curvesIdx);
        }
    }

    if ( pIdx.isValid() ) {
        // Add Markers parent to either Plot or Curve
        QString pTag = _bookModel()->data(pIdx).toString();
        QStandardItem *pItem = _bookModel()->itemFromIndex(pIdx);
        if ( pTag == "Curve" ) {
            if (!_bookModel()->isChildIndex(pIdx,"Curve","Markers")) {
                _bookModel()->addChild(pItem, "Markers","");
            }
        } else if ( pTag == "Plot" ) {
            if (!_bookModel()->isChildIndex(pIdx,"Plot","Markers")) {
                _bookModel()->addChild(pItem, "Markers","");
            }
        }

        QModelIndex markersIdx = _bookModel()->getIndex(pIdx,"Markers","");
        QModelIndexList markerIdxs = _bookModel()->getIndexList(markersIdx,
                                                            "Marker","Markers");

        bool isFound = false;
        foreach ( QModelIndex markerIdx, markerIdxs ) {
            double markerTime = _bookModel()->getDataDouble(markerIdx,
                                                         "MarkerTime","Marker");
            int markerTimeIdx = _bookModel()->getDataInt(markerIdx,
                                                      "MarkerTimeIdx","Marker");
            if ( markerTime == time && markerTimeIdx == timeIdx ) {
                // Time already marked, just reset label
                QModelIndex markerLabelIdx = _bookModel()->getDataIndex(
                                             markerIdx,"MarkerLabel", "Marker");
                model()->setData(markerLabelIdx,label);
                isFound = true;
                break;
            }
        }
        if ( !isFound ) {
            QStandardItem* markersItem =_bookModel()->itemFromIndex(markersIdx);
            QStandardItem* markerItem = _bookModel()->addChild(markersItem,
                                                               "Marker","");
            _bookModel()->addChild(markerItem,"MarkerTime", time);
            _bookModel()->addChild(markerItem,"MarkerTimeIdx", timeIdx);
            _bookModel()->addChild(markerItem,"MarkerLabel", label);
        }
    }
}

void CurvesView::paintEvent(QPaintEvent *event)
{
#if 0
    Q_UNUSED(event);

    if ( !model() ) return;

    QRect R = viewport()->rect();
    QRect RG =  R;
    RG.moveTo(viewport()->mapToGlobal(RG.topLeft()));
    QRect  C = _curvesView->viewport()->rect();
    C.moveTo(_curvesView->viewport()->mapToGlobal(C.topLeft()));
    QRectF M = _bookModel()->getPlotMathRect(rootIndex());
    QPainter painter(viewport());
    CurvesLayoutItem layoutItem(_bookModel(),rootIndex(),0);
    layoutItem.paint(&painter,R,RG,C,M);
#endif

#if 1
    Q_UNUSED(event);

    if ( !model() ) return;

    QModelIndex curvesIdx = _bookModel()->getIndex(rootIndex(),"Curves","Plot");
    int nCurves = model()->rowCount(curvesIdx);

    QPainter painter(viewport());
    painter.save();

    painter.setRenderHint(QPainter::Antialiasing);

    QTransform T = _coordToPixelTransform();
    double ptSizeCurve = 0.0;
    QPen pen;
    pen.setWidthF(ptSizeCurve);
    painter.setPen(pen);

    // Draw curves
    if ( nCurves == 2 ) {
        QString plotPresentation = _bookModel()->getDataString(rootIndex(),
                                                     "PlotPresentation","Plot");
        if ( plotPresentation.isEmpty() ) {
            plotPresentation = _bookModel()->getDataString(QModelIndex(),
                                                           "Presentation");
        }

        // Plot background
        QModelIndex pageIdx = rootIndex().parent().parent();
        QColor bg = _bookModel()->pageBackgroundColor(pageIdx);
        painter.fillRect(viewport()->rect(),bg);

        _paintGrid(painter,rootIndex());
        _paintHLines(painter,rootIndex());

        if ( plotPresentation == "compare" ) {
            _paintCoplot(T,painter,pen);
        } else if ( plotPresentation == "error" ) {
            _paintErrorplot(T,painter,pen,rootIndex());
        } else if ( plotPresentation == "error+compare" ) {
            _paintCoplot(T,painter,pen);
            _paintErrorplot(T,painter,pen,rootIndex()); // overlay err on coplot
        } else {
            fprintf(stderr,"koviz [bad scoobs]: paintEvent() : "
                           "PlotPresentation=\"%s\" not recognized.\n",
                           plotPresentation.toLatin1().constData());
            exit(-1);
        }
    } else {
        _paintCoplot(T,painter,pen);
    }

    // Restore the painter state off the painter stack
    painter.restore();

    // Draw legend (if needed)
    _paintCurvesLegend(viewport()->rect(),curvesIdx,painter);

    // Draw markers
    _paintMarkers(painter,rootIndex());

    if ( _isMeasure ) {
        painter.drawEllipse(_mousePressPos,3,3);
        painter.drawEllipse(_mouseCurrPos,2,2);
        painter.drawLine(_mousePressPos,_mouseCurrPos);
    }
#endif
}


void CurvesView::_paintCoplot(const QTransform &T,QPainter &painter,QPen &pen)
{
    Q_UNUSED(pen);

    if ( !_pixmap ) return;

    painter.save();

    // Draw pixmap of all curves
    QTransform I;
    painter.setTransform(I);
    painter.drawPixmap(viewport()->rect(),*_pixmap);

    // If curve is selected
    QModelIndex gpidx = currentIndex().parent().parent();
    QString tag = model()->data(currentIndex()).toString();
    if ( currentIndex().isValid() && tag == "Curve" && gpidx == rootIndex()) {

        // Lighten unselected curves to semi-transparent bg
        QModelIndex pageIdx = rootIndex().parent().parent();
        QColor bg = _bookModel()->pageBackgroundColor(pageIdx);
        bg.setAlpha(190);
        painter.fillRect(viewport()->rect(),bg);

        // Since grid and hlines too light with semi-transparent bg, paint them
        _paintGrid(painter,rootIndex());
        _paintHLines(painter,rootIndex());

        // Paint curve (possibly with linestyle, symbols etc.)
        _paintCurve(currentIndex(),T,painter,true);
    }

    painter.restore();
}

void CurvesView::_paintCurve(const QModelIndex& curveIdx,
                             const QTransform& T,
                             QPainter& painter, bool isHighlight)
{
    painter.save();
    QPen origPen = painter.pen();

    CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);

    if ( curveModel ) {

        // Line color
        QPen pen;
        pen.setWidth(0);
        QColor color(_bookModel()->getDataString(curveIdx,
                                                 "CurveColor","Curve"));
        if ( isHighlight ) {
            QModelIndex pageIdx = curveIdx.parent().parent().parent().parent();
            QColor bg = _bookModel()->pageBackgroundColor(pageIdx);
            if ( bg.lightness() < 128 ) {
                color = color.lighter(120);
            } else {
                color = color.darker(200);
            }
        }
        pen.setColor(color);

        // Line style pattern
        QString linestyle =  _bookModel()->getDataString(curveIdx,
                                                      "CurveLineStyle","Curve");
        QVector<qreal> pattern = _bookModel()->getLineStylePattern(linestyle);
        pen.setDashPattern(pattern);

        // Set pen
        painter.setPen(pen);

        // Get painter path
        QPainterPath* path = _bookModel()->getPainterPath(curveIdx);

        // Get plot scale
        QModelIndex plotIdx = curveIdx.parent().parent();
        QString plotXScale = _bookModel()->getDataString(plotIdx,
                                                         "PlotXScale","Plot");
        QString plotYScale = _bookModel()->getDataString(plotIdx,
                                                         "PlotYScale","Plot");

        // Scale transform
        // If logscale, scale/bias done in _createPainterPath
        double xs = 1.0;
        double ys = 1.0;
        double xb = 0.0;
        double yb = 0.0;
        if ( plotXScale == "linear" ) {
            xs = _bookModel()->getDataDouble(curveIdx,"CurveXScale","Curve");
            xb = _bookModel()->getDataDouble(curveIdx,"CurveXBias","Curve");
        }
        if ( plotYScale == "linear" ) {
            ys = _bookModel()->getDataDouble(curveIdx,"CurveYScale","Curve");
            yb = _bookModel()->getDataDouble(curveIdx,"CurveYBias","Curve");
        }
        QTransform Tscaled(T);
        Tscaled = Tscaled.scale(xs,ys);
        Tscaled = Tscaled.translate(xb/xs,yb/ys);
        painter.setTransform(Tscaled);

        QRectF cbox = path->boundingRect();
        if ( cbox.height() == 0.0 && path->elementCount() > 0 ) {

            QString label;
            if ( path->elementCount() == 1 ) {
                // "Point=(x,y)" label if curve has single point
                QPainterPath::Element el = path->elementAt(0);
                double x = el.x*xs+xb;
                if (plotXScale=="log") {
                    x = pow(10,x) ;
                }
                double y = el.y*ys+yb;
                if (plotYScale=="log") {
                    y = pow(10,y) ;
                }
                label = QString("Point=(%1,%2)").arg(x).arg(y);
            } else {
                // "Flatline=#" label if curve is flat (constant)
                double y = cbox.y()*ys+yb;
                if (plotYScale=="log") {
                    y = pow(10,y) ;
                }
                label = QString("Flatline=%1").arg(y,0,'g',15);
            }

            QRectF tbox = Tscaled.mapRect(cbox);
            QTransform I;
            painter.setTransform(I);
            double top = tbox.y()-fontMetrics().ascent();
            QPoint drawPt;
            if ( top >= 0 ) {
                // Draw label over curve
                drawPt = tbox.topLeft().toPoint()-QPoint(0,5);
            } else {
                // Draw label under curve since it would drawn off page
                drawPt = tbox.topLeft().toPoint()+
                                QPoint(0,fontMetrics().ascent()) + QPoint(0,5);
            }
            QRect labelRect = painter.fontMetrics().boundingRect(label);
            labelRect.moveTo(tbox.topLeft().toPoint());
            if ( labelRect.right() > painter.viewport().right() ) {
                // Shift label to left of point so it stays in viewport
                drawPt = drawPt - QPoint(labelRect.width(),0);
            }
            painter.drawText(drawPt,label);
            painter.setTransform(Tscaled);

        } else if ( path->elementCount() == 0 ) {
            // Empty plot
            QTransform I;
            painter.setTransform(I);
            QString lbl("Empty");
            QRect bb = fontMetrics().boundingRect(lbl);
            QRect R = viewport()->rect();
            painter.drawText(R.center()+QPointF(-bb.width()/2,0),lbl);
            painter.setTransform(Tscaled);
        }

        // Line style
        QString lineStyle = _bookModel()->getDataString(curveIdx,
                                                      "CurveLineStyle","Curve");
        lineStyle = lineStyle.toLower();

        // Get plot math rect
        QRectF M = _bookModel()->getPlotMathRect(plotIdx);

        // Draw curve!
        if ( lineStyle == "thick_line" || lineStyle == "x_thick_line" ) {
            // The transform cannot be used when drawing thick lines
            QTransform I;
            painter.setTransform(I);
            double w = pen.widthF();
            if ( lineStyle == "thick_line" ) {
                pen.setWidth(3.0);
            } else if ( lineStyle == "x_thick_line" ) {
                pen.setWidthF(5.0);
            } else {
                fprintf(stderr, "koviz [bad scoobs]: "
                                "CurvesView::_paintCurve: bad linestyle\n");
                exit(-1);
            }
            painter.setPen(pen);
            QPointF pLast;
            for ( int i = 0; i < path->elementCount(); ++i ) {
                QPainterPath::Element el = path->elementAt(i);
                QPointF p(el.x,el.y);
                p = Tscaled.map(p);
                if  ( i > 0 ) {
                    painter.drawLine(pLast,p);
                }
                pLast = p;
            }
            pen.setWidthF(w);
            painter.setPen(pen);
            painter.setTransform(Tscaled);
        } else if ( lineStyle == "scatter" ) {
            QTransform I;
            painter.setTransform(I);
            double w = pen.widthF();
            pen.setWidthF(1.5);
            painter.setPen(pen);
            QBrush origBrush = painter.brush();
            QBrush brush(Qt::SolidPattern);
            brush.setColor(color);
            painter.setBrush(brush);
            double r = pen.widthF();
            for ( int i = 0; i < path->elementCount(); ++i ) {
                QPainterPath::Element el = path->elementAt(i);
                QPointF p(el.x,el.y);
                p = Tscaled.map(p);
                painter.drawEllipse(p,r,r);
            }
            pen.setWidthF(w);
            painter.setPen(pen);
            painter.setBrush(origBrush);
            painter.setTransform(Tscaled);
        } else if ( M.height() < 1.0e-17) {
            // When zoomed way in, draw curve point by point, otherwise
            // Qt (I think) has issues where the pixmap curve doesn't match
            // the painter.drawPath(path) curve when selecting curves
            QTransform I;
            painter.setTransform(I);
            painter.setPen(pen);
            QPointF pLast;
            for ( int i = 0; i < path->elementCount(); ++i ) {
                QPainterPath::Element el = path->elementAt(i);
                QPointF p(el.x,el.y);
                p = Tscaled.map(p);
                if  ( i > 0 ) {
                    painter.drawLine(pLast,p);
                }
                pLast = p;
            }
            painter.setPen(pen);
            painter.setTransform(Tscaled);
        } else {
            painter.drawPath(*path);
        }

        // Draw symbols on curve (if there are any)
        QString symbolStyle = _bookModel()->getDataString(curveIdx,
                                               "CurveSymbolStyle", "Curve");
        symbolStyle = symbolStyle.toLower();
        if ( !symbolStyle.isEmpty() && symbolStyle != "none" ) {
            pattern.clear();
            pen.setDashPattern(pattern); // plain lines for drawing symbols
            QTransform I;
            painter.setTransform(I);
            double w = pen.widthF();
            pen.setWidthF(0.0);
            painter.setPen(pen);
            QPointF pLast;
            for ( int i = 0; i < path->elementCount(); ++i ) {
                QPainterPath::Element el = path->elementAt(i);
                QPointF p(el.x,el.y);
                p = Tscaled.map(p);
                if ( i > 0 ) {
                    double r = 32.0;
                    double x = pLast.x()-r/2.0;
                    double y = pLast.y()-r/2.0;
                    QRectF R(x,y,r,r);
                    if ( R.contains(p) ) {
                        continue;
                    }
                }

                __paintSymbol(p,symbolStyle,painter);

                pLast = p;
            }
            pen.setWidthF(w);
            painter.setPen(pen);
            painter.setTransform(Tscaled);
        }

        QString symbolEnd = _bookModel()->getDataString(curveIdx,
                                                     "CurveSymbolEnd", "Curve");
        if ( !symbolEnd.isEmpty() && symbolEnd != "none" ) {
            int n = path->elementCount();
            if ( n > 0 ) {
                QTransform I;
                painter.setTransform(I);
                QPainterPath::Element el = path->elementAt(n-1);
                QPointF p(el.x,el.y);
                p = Tscaled.map(p);
                __paintSymbol(p,symbolEnd,painter);
            }
        }

    }
    painter.setPen(origPen);
    painter.restore();
}

int CurvesView::_idxAtTimeBinarySearch(QPainterPath* path,
                                       int low, int high, double time)
{
        if (high <= 0 ) {
                return 0;
        }
        if (low >= high) {
                return ( path->elementAt(high).x > time ) ? high-1 : high;
        } else {
                int mid = (low + high)/2;
                if (time == path->elementAt(mid).x ) {
                        return mid;
                } else if ( time < path->elementAt(mid).x ) {
                        return _idxAtTimeBinarySearch(path,
                                                      low, mid-1, time);
                } else {
                        return _idxAtTimeBinarySearch(path,
                                                      mid+1, high, time);
                }
        }
}

void CurvesView::_keyPressPeriod()
{
    // If curve is selected
    QModelIndex gpidx = currentIndex().parent().parent();
    QString tag = model()->data(currentIndex()).toString();
    if ( currentIndex().isValid() && tag == "Curve" && gpidx == rootIndex()) {

        QModelIndex curveIdx = currentIndex();

        CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
        curveModel->map();

        // Calculate liveCoord based on model liveCoordTime
        double xs = _bookModel()->getDataDouble(curveIdx,"CurveXScale","Curve");
        double ys = _bookModel()->getDataDouble(curveIdx,"CurveYScale","Curve");
        double xb = _bookModel()->getDataDouble(curveIdx,"CurveXBias","Curve");
        double yb = _bookModel()->getDataDouble(curveIdx,"CurveYBias","Curve");
        double xus = 1.0;
        double xub = 0.0;
        QString bookXUnit = _bookModel()->getDataString(curveIdx,
                                                        "CurveXUnit","Curve");
        if ( !bookXUnit.isEmpty() && bookXUnit != "--" ) {
            QString loggedXUnit = curveModel->x()->unit();
            xus = Unit::scale(loggedXUnit, bookXUnit);
            xub = Unit::bias(loggedXUnit, bookXUnit);
        }
        double yus = 1.0;
        double yub = 0.0;
        QString bookYUnit = _bookModel()->getDataString(curveIdx,
                                                        "CurveYUnit","Curve");
        if ( !bookYUnit.isEmpty() && bookYUnit != "--" ) {
            QString loggedYUnit = curveModel->y()->unit();
            yus = Unit::scale(loggedYUnit, bookYUnit);
            yub = Unit::bias(loggedYUnit, bookYUnit);
        }
        QModelIndex liveIdx = _bookModel()->getDataIndex(QModelIndex(),
                                                         "LiveCoordTime");
        double liveTime = model()->data(liveIdx).toDouble();
        int i = 0;
        if ( curveModel->x()->name() == curveModel->t()->name() ) {
            // xub not used below since time units have no bias
            i = curveModel->indexAtTime((liveTime-xb)/xs/xus);
        } else {
            // e.g. ball xy curve where x is position[0]
            i = curveModel->indexAtTime(liveTime);
        }
        ModelIterator* it = curveModel->begin();
        double x = it->at(i)->x();
        x = xus*x + xub;
        x = xs*x + xb;
        double y = it->at(i)->y();
        y = yus*y + yub;
        y = ys*y + yb;
        QPointF coord(x,y);
        delete it;

        QString xStr = _format(coord.x());
        QString yStr = _format(coord.y());
        QString coordStr = QString("coord=(%1,%2)").arg(xStr).arg(yStr);
        fprintf(stderr,"%-40s",coordStr.toLatin1().constData());

        if ( _isLastPoint ) {
            QPointF dPoint = coord-_lastPoint;
            QString dX = _format(dPoint.x());
            QString dY = _format(dPoint.y());
            QString xUnit = _bookModel()->getDataString(curveIdx,
                                                        "CurveXUnit","Curve");
            QString yUnit = _bookModel()->getDataString(curveIdx,
                                                        "CurveYUnit","Curve");
            QString xStr = QString("dx=%1 {%2}").arg(dX).arg(xUnit);
            QString yStr = QString("dy=%1 {%2}").arg(dY).arg(yUnit);
            fprintf(stderr,"%-25s %-25s\n",
                    xStr.toLatin1().constData(),
                    yStr.toLatin1().constData());
        } else {
            fprintf(stderr, "\n");
        }

        curveModel->unmap();
        _isLastPoint = true;
        _lastPoint = coord;
    }
}

void CurvesView::_keyPressArrow(const Qt::ArrowType& arrow)
{
    if ( arrow != Qt::LeftArrow && arrow != Qt::RightArrow ) {
        fprintf(stderr, "koviz [bad scoobs]: "
                        "CurvesView::_keyPressArrow(%d)\n",arrow);
        exit(-1);
    }

    // If curve is selected
    QModelIndex gpidx = currentIndex().parent().parent();
    QString tag = model()->data(currentIndex()).toString();
    if ( currentIndex().isValid() &&
         ((tag == "Curve" && gpidx == rootIndex()) ||
          (tag == "Plot" && currentIndex() == rootIndex())) ) {
        QModelIndex idx = currentIndex();
        if ( arrow == Qt::LeftArrow ) {
            _bookModel()->liveTimePrev(idx);
        } else if ( arrow == Qt::RightArrow ) {
            _bookModel()->liveTimeNext(idx);
        }
    }
}

void CurvesView::_paintErrorplot(const QTransform &T,
                                 QPainter &painter, const QPen &pen,
                                 const QModelIndex& plotIdx)
{
    painter.save();

    QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
    QPainterPath* errorPath = _bookModel()->getCurvesErrorPath(curvesIdx);

    QRectF ebox = errorPath->boundingRect();
    QPen ePen(pen);
    if ( ebox.height() == 0.0 && ebox.y() == 0.0 ) {
        // Color green if error plot is flatline zero
        ePen.setColor(_bookModel()->flatLineColor());
    } else {
        ePen.setColor(_bookModel()->errorLineColor());
    }
    painter.setPen(ePen);
    if ( ebox.height() == 0.0 && errorPath->elementCount() > 0 ) {
        // Flatline
        QString label;
        QString plotYScale = _bookModel()->getDataString(plotIdx,
                                                         "PlotYScale","Plot");
        double error = ebox.y();
        if ( plotYScale == "log" ) {
            error = pow(10,error) ;
        }
        if ( ebox.width() == 0 ) {
            QString plotXScale = _bookModel()->getDataString(plotIdx,
                                                           "PlotXScale","Plot");
            double x = ebox.x();
            if ( plotXScale == "log" ) {
                x = pow(10,x) ;
            }
            label = QString("Flatpoint=(%1,%2)").arg(x,0,'g').arg(error,0,'g');
        } else {
            label = QString("Flatline=%1").arg(error,0,'g', 15);
        }
        QTransform I;
        painter.setTransform(I);
        QRectF tbox = T.mapRect(ebox);
        double top = tbox.y()-fontMetrics().ascent();
        QPoint drawPt;
        if ( top >= 0 ) {
            // Draw label over curve
            drawPt = tbox.topLeft().toPoint()-QPoint(0,5);
        } else {
            // Draw label under curve since it would be drawn off page
            drawPt = tbox.topLeft().toPoint() +
                     QPoint(0,fontMetrics().ascent()) + QPoint(0,5);
        }
        painter.drawText(drawPt,label);
    } else if ( errorPath->elementCount() == 0 ) {
        // Empty plot
        QTransform I;
        painter.setTransform(I);
        QString lbl("Empty");
        QRect bb = fontMetrics().boundingRect(lbl);
        QRect R = viewport()->rect();
        painter.drawText(R.center()+QPointF(-bb.width()/2,0),lbl);
    }
    painter.setTransform(T);
    painter.drawPath(*errorPath);

    delete errorPath;

    painter.setPen(pen);
    painter.restore();
}

QSize CurvesView::minimumSizeHint() const
{
    QSize s;
    return s;
}

QSize CurvesView::sizeHint() const
{
    QSize s;
    return s;
}

void CurvesView::dataChanged(const QModelIndex &topLeft,
                             const QModelIndex &bottomRight,
                             const QVector<int> &roles)
{
    Q_UNUSED(roles);
    Q_UNUSED(bottomRight);

    if ( topLeft.column() != 1 ) return;

    QString tag = model()->data(topLeft.sibling(topLeft.row(),0)).toString();

    if ( tag == "PlotMathRect" && topLeft.parent() == rootIndex() ) {

        QRectF M = model()->data(topLeft).toRectF();

        if ( M.size().width() > 0 && M.size().height() != 0 && _lastM != M ) {
            if ( _pixmap ) {
                delete _pixmap;
            }
            _pixmap = _createLivePixmap();
        }

        _lastM = M;  // Saved so that pixmap is not recreated if M unchanged

    } else if ( tag == "PlotMathRect" && topLeft.parent() != rootIndex() ) {
        // Another plot has changed its PlotMathRect.
        // Synchronize this plot's PlotMathRect with the changed one
        // to keep zoom/pan synchronized across plots.

        // Only synchronize when x variables are time (normal case).
        if ( _bookModel()->isXTime(topLeft.parent()) &&
             _bookModel()->isXTime(rootIndex()) ) {
            QRectF M = model()->data(topLeft).toRectF();
            QModelIndex plotRectIdx = _bookModel()->getDataIndex(rootIndex(),
                                                         "PlotMathRect","Plot");
            QRectF R = model()->data(plotRectIdx).toRectF();

            QString R_PlotXScale = _bookModel()->getDataString(rootIndex(),
                                                               "PlotXScale",
                                                               "Plot");
            QString M_PlotXScale = _bookModel()->getDataString(topLeft.parent(),
                                                               "PlotXScale",
                                                               "Plot");
            if ( M_PlotXScale == "log" && R_PlotXScale == "linear" ) {
                M.setLeft(pow(10,M.left()));
                M.setRight(pow(10,M.right()));
            } else if ( M_PlotXScale == "linear" && R_PlotXScale == "log") {
                if ( M.left() != 0.0 ) {
                    M.setLeft(log10(M.left()));
                }
                if ( M.right() != 0.0 ) {
                    M.setRight(log10(M.right()));
                }
            }
            if ( M.left() != R.left() || M.right() != R.right() ) {
                R.setLeft(M.left());
                R.setRight(M.right());
                double plotXMinRange = _bookModel()->getDataDouble(rootIndex(),
                                                        "PlotXMinRange","Plot");
                double plotXMaxRange = _bookModel()->getDataDouble(rootIndex(),
                                                        "PlotXMaxRange","Plot");
                if ( R.left() >= plotXMinRange && R.right() <= plotXMaxRange ) {
                    // Set R if within PlotXMin/MaxRange
                    _bookModel()->setPlotMathRect(R,rootIndex());
                }
            }
        }
    } else if ( topLeft.parent().parent().parent() == rootIndex() ) {
        if ( tag == "CurveXBias" ) {
            if ( _pixmap ) {
                delete _pixmap;
            }
            _pixmap = _createLivePixmap();
        } else if ( tag == "CurveYBias" ) {
            if ( _pixmap ) {
                delete _pixmap;
            }
            _pixmap = _createLivePixmap();
        } else if ( tag == "CurveColor") {
            if ( _pixmap ) {
                delete _pixmap;
            }
            _pixmap = _createLivePixmap();
        } else if ( tag == "CurveData") {
            if ( _pixmap ) {
                delete _pixmap;
            }
            _pixmap = _createLivePixmap();
        }
    } else if ( topLeft.parent() == rootIndex() ) {
        if ( tag == "PlotXScale" || tag == "PlotYScale" ) {
            if ( _pixmap ) {
                delete _pixmap;
            }
            _pixmap = _createLivePixmap();
            QModelIndex curvesIdx = _bookModel()->getIndex(rootIndex(),
                                                           "Curves","Plot");
            QRectF bbox = _bookModel()->calcCurvesBBox(curvesIdx);
            _bookModel()->setPlotMathRect(bbox,rootIndex());
        }
    }

    viewport()->update();
    update();
}

QPixmap* CurvesView::_createLivePixmap()
{
    if ( viewport()->rect().size().width() == 0 ||
         viewport()->rect().size().height() == 0 ) {
        return 0;
    }
    bool isCurves  = _bookModel()->isChildIndex(rootIndex(),"Plot","Curves");
    if ( !isCurves ) {
        return 0;
    }
    QString pres = _bookModel()->getDataString(rootIndex(),
                                               "PlotPresentation","Plot");
    if ( pres == "error" ) {
        // Pixmaps are only used to optimize coplots
        return 0;
    }
    QRectF M = _mathRect();
    if ( M.width() == 0 || M.height() == 0 ) {
        return 0;
    }

    QPixmap* livePixmap = new QPixmap(viewport()->rect().size());

    QPainter painter(livePixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QModelIndex pageIdx = rootIndex().parent().parent();
    QColor bg = _bookModel()->pageBackgroundColor(pageIdx);
    painter.fillRect(viewport()->rect(),bg);

    _paintGrid(painter, rootIndex());
    _paintHLines(painter,rootIndex());

    QTransform T = _coordToPixelTransform();
    QModelIndex curvesIdx = _bookModel()->getIndex(rootIndex(),"Curves","Plot");
    int rc = model()->rowCount(curvesIdx);
    for ( int i = 0; i < rc; ++i ) {
        QModelIndex curveIdx = model()->index(i,0,curvesIdx);
        _paintCurve(curveIdx,T,painter,false);
    }

    return livePixmap;
}

QString CurvesView::_format(double d)
{
    QString s;
    s = s.asprintf("%.9g",d);
    QVariant v(s);
    double x = v.toDouble();
    double e = qAbs(x-d);
    if ( e > 1.0e-9 ) {
        s = s.asprintf("%.9lf",d);
    }
    return s;
}

// TODO: This thing does nothing!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void CurvesView::rowsInserted(const QModelIndex &pidx, int start, int end)
{
    if ( pidx.parent().parent() != rootIndex() ) return; // not my plot

        for ( int i = start; i <= end; ++i ) {
            QModelIndex curveIdx = model()->index(i,0,pidx);
            QModelIndex curveDataIdx = model()->index(i,1,pidx);
            QString name = model()->data(curveIdx).toString();
            if ( name == "CurveData" ) {
                QVariant v = model()->data(curveDataIdx);
                CurveModel* curveModel = QVariantToPtr<CurveModel>::convert(v);
                if ( curveModel ) {
                    //_setPlotMathRect(_currBBox);
                }
                break;
            }
        }
}

QPainterPath CurvesView::_sinPath()
{
    // Make sin(t) t=[0,2*pi]
    QPainterPath path;
    path.moveTo(0,0);
    double t = 0;
    double dt = 0.001;
    while (1) {
        t += dt;
        if ( t > 2*M_PI-0.001 ) {
            t = 2*M_PI;
        }
        path.lineTo(t,sin(t));
        if ( t == 2*M_PI ) break;
    }
    return path;
}

QPainterPath CurvesView::_stepPath()
{
    QPainterPath path;
    path.moveTo(0,0.0);
    double t = 0;
    double dt = 0.01;
    while (1) {
        path.lineTo(t,floor(t));
        if ( t > 10.0 ) break;
        t += dt;
    }
    return path;
}

void CurvesView::mousePressEvent(QMouseEvent *event)
{
    _mouseCurrPos = event->pos();
    if (  event->button() == _buttonSelectAndPan ) {
        _mousePressPos = event->pos();
        _mousePressMathRect = _mathRect();
        if ( currentIndex().isValid() ) {
            QString tag = model()->data(currentIndex()).toString();
            QString presentation = _bookModel()->getDataString(rootIndex(),
                                                  "PlotPresentation","Plot");
            if ( (tag == "Curve" &&
                  (presentation == "compare" || presentation.isEmpty())) ) {

                CurveModel* curveModel =
                                    _bookModel()->getCurveModel(currentIndex());
                if ( curveModel ) {
                    QModelIndex curveIdx = currentIndex();
                    _mousePressXBias = _bookModel()->getDataDouble(curveIdx,
                                                          "CurveXBias","Curve");
                    _mousePressYBias = _bookModel()->getDataDouble(curveIdx,
                                                          "CurveYBias","Curve");
                }
            }
        }
        event->ignore();
    } else if (  event->button() == _buttonRubberBandZoom ) {
        event->ignore();
    } else if ( event->button() == _buttonResetView ) {
        QModelIndex curvesIdx = _bookModel()->getIndex(rootIndex(),
                                                       "Curves","Plot");
        QRectF bbox = _bookModel()->calcCurvesBBox(curvesIdx);
        _bookModel()->setPlotMathRect(bbox,rootIndex());
        viewport()->update();
    }
}

void CurvesView::mouseReleaseEvent(QMouseEvent *event)
{
    bool isShift = false;
    Qt::KeyboardModifiers keymods = event->modifiers();
    if ( keymods & Qt::ShiftModifier ) {
        isShift = true;
    }

    if ( _isMeasure ) {
        viewport()->update();
        _isMeasure = false;
    }

    if (  event->button() == _buttonSelectAndPan && isShift ) {
        // Toggle between single/multi views when clicking with shift key
        event->ignore();
    } else if (  event->button() == _buttonSelectAndPan && !isShift ) {
        double x0 = _mousePressPos.x();
        double y0 = _mousePressPos.y();
        double x1 = event->pos().x();
        double y1 = event->pos().y();
        double d = qSqrt((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0));
        QString presentation = _bookModel()->getDataString(rootIndex(),
                                                   "PlotPresentation","Plot");
        if ( d < 10 && (presentation == "compare" || presentation.isEmpty()) ) {
            // d < 10, to hopefully catch click and not a drag
            QModelIndex curveIdx = _chooseCurveNearMousePoint(event->pos());
            if ( curveIdx.isValid() ) {
                // Curve found in small box around mouse click!
                if ( currentIndex() == curveIdx ) {
                    // If current curve clicked again - "deselect"
                    setCurrentIndex(QModelIndex());
                } else {
                    // If curve other than current clicked - "select" it
                    selectionModel()->setCurrentIndex(curveIdx,
                                                 QItemSelectionModel::NoUpdate);
                }
            } else if ( !curveIdx.isValid() &&  currentIndex().isValid() ) {
                // click off curves, current unset i.e. "deselect curve"
                setCurrentIndex(QModelIndex());
            } else if ( !curveIdx.isValid() &&  !currentIndex().isValid() ) {
                // click off curves when nothing selected - do nothing
            }
        } else if ( d < 10 && presentation == "error" ) {
            bool isMouseNearCurve = _isErrorCurveNearMousePoint(event->pos());
            if ( currentIndex().isValid() && isMouseNearCurve ) {
                setCurrentIndex(QModelIndex());  // toggle to deselect
            } else if ( !currentIndex().isValid() && isMouseNearCurve ) {
                QModelIndex plotIdx = rootIndex();
                if ( !_bookModel()->isIndex(plotIdx,"Plot") ) {
                    fprintf(stderr, "koviz [bad scoobs]: "
                                    "CurvesView::mouseReleaseEvent():1\n");
                    exit(-1);
                }
                setCurrentIndex(plotIdx);  // Select curve
            } else if ( currentIndex().isValid() && !isMouseNearCurve ) {
                // click off curves when curve selected -> deselect
                setCurrentIndex(QModelIndex());  // toggle to deselect
            } else if ( !currentIndex().isValid() && !isMouseNearCurve ) {
                // click off curves when nothing selected - do nothing
            } else {
                fprintf(stderr, "koviz [bad scoobs]: "
                                 "CurvesView::mouseReleaseEvent():2\n");
                exit(-1);
            }
        } else {
            //event->ignore(); // pass event to parent view for stretch,zoom etc
        }
    } else if (  event->button() == _buttonResetView ) {
        event->ignore();
    } else if ( event->button() == _buttonRubberBandZoom ) {
        event->ignore();
    }
}

// Choose first curve found within 12 pixel square about pt
QModelIndex CurvesView::_chooseCurveNearMousePoint(const QPoint &pt)
{
    QModelIndex idx;

    QImage img(viewport()->rect().size(),QImage::Format_Mono);

    QPainter painter(&img);
    QPen penBlack(img.colorTable().at(0));
    penBlack.setWidth(0);
    painter.setPen(penBlack);

    QTransform T = _coordToPixelTransform();  // _paintCurve sets painter tform

    int s = 12; // side length of small square around mouse click
    QRectF R(pt.x()-s/2,pt.y()-s/2,s,s);

    // Speed up test by drawing clipped to small square
    painter.setClipRect(R);

    QRectF W = viewport()->rect();
    QRectF M = _mathRect();
    double a = M.width()/W.width();
    double b = M.height()/W.height();
    double c = M.x() - a*W.x();
    double d = M.y() - b*W.y();
    QTransform U( a,    0,
                  0,    b, /*+*/ c,    d);
    M = U.mapRect(R);

    QString plotXScale = _bookModel()->getDataString(rootIndex(),
                                                     "PlotXScale","Plot");
    QString plotYScale = _bookModel()->getDataString(rootIndex(),
                                                     "PlotYScale","Plot");

    QModelIndex curvesIdx = _bookModel()->getIndex(rootIndex(),"Curves","Plot");
    int rc = model()->rowCount(curvesIdx);
    for ( int i = rc-1; i >= 0; --i ) {  // check curves from top to bottom

        // fill image with white (see help for QImage::fill(int))
        img.fill(1);

        // Get underlying path that goes with curve
        QModelIndex curveIdx = model()->index(i,0,curvesIdx);
        QPainterPath* path = _bookModel()->getPainterPath(curveIdx);
        if ( !path ) {
            continue;
        }

        // Get xy scale/bias (logscale path is already biased/scaled)
        double xs = 1.0;
        double xb = 0.0;
        double ys = 1.0;
        double yb = 0.0;
        if ( plotXScale == "linear" ) {
            xs = _bookModel()->getDataDouble(curveIdx,"CurveXScale","Curve");
            xb = _bookModel()->getDataDouble(curveIdx,"CurveXBias","Curve");
        }
        if ( plotYScale == "linear" ) {
            ys = _bookModel()->getDataDouble(curveIdx,"CurveYScale","Curve");
            yb = _bookModel()->getDataDouble(curveIdx,"CurveYBias","Curve");
        }
        QTransform Tscaled(T);
        Tscaled = Tscaled.scale(xs,ys);
        Tscaled = Tscaled.translate(xb/xs,yb/ys);
        painter.setTransform(Tscaled);

        // Ignore paths whose bounding box does not intersect
        // math click rect. This speeds up a special case of
        // selecting a spike which falls outside most other curves
        if ( xs == 1.0 && ys == 1.0 && xb == 0 && yb == 0 ) {
            // TODO: Lazily checking for xs==ys==1.0 because of scaling issues
            if ( !path->intersects(M) ) {
                continue;
            }
        }

        // Draw curve onto monochrome image (clipped to small square)
        painter.drawPath(*path);

        // Check, pixel by pixel, to see if the curve
        // is in small rectangle around mouse click
        bool isFound = false;
        for ( int x = pt.x()-s/2; x < pt.x()+s/2; ++x ) {
            if ( x < 0 || x >= img.width() ) {
                continue;
            }
            for ( int y = pt.y()-s/2; y < pt.y()+s/2; ++y ) {
                if ( y < 0 || y >= img.height() ) {
                    continue;
                }
                QRgb pix = img.pixel(x,y);
                if ( qRed(pix) == 0 ) {
                    // qRed(pix) is arbitrary. Pen is black and has no red.
                    isFound = true;
                    break;
                }
            }
            if ( isFound ) break;
        }
        if ( isFound ) {
            idx = curveIdx;  // choose first curve inside rect and bail
            break;
        }


    }

    return idx;
}

bool CurvesView::_isErrorCurveNearMousePoint(const QPoint &pt)
{
    bool isNear = false;

    QImage img(viewport()->rect().size(),QImage::Format_Mono);

    QPainter painter(&img);
    QPen penBlack(img.colorTable().at(0));
    penBlack.setWidth(0);
    painter.setPen(penBlack);

    QTransform T = _coordToPixelTransform();  // _paintCurve sets painter tform

    int s = 12; // side length of small square around mouse click
    QRectF R(pt.x()-s/2,pt.y()-s/2,s,s);

    // Speed up test by drawing clipped to small square
    painter.setClipRect(R);

    QRectF W = viewport()->rect();
    QRectF M = _mathRect();
    double a = M.width()/W.width();
    double b = M.height()/W.height();
    double c = M.x() - a*W.x();
    double d = M.y() - b*W.y();
    QTransform U( a,    0,
                  0,    b, /*+*/ c,    d);
    M = U.mapRect(R);

    QModelIndex curvesIdx = _bookModel()->getIndex(rootIndex(),"Curves","Plot");
    QPainterPath* path = _bookModel()->getCurvesErrorPath(curvesIdx);
    if ( path ) {

        // fill image with white (see help for QImage::fill(int))
        img.fill(1);

        painter.setTransform(T);

        if ( path->intersects(M) ) {

            // Draw curve onto monochrome image (clipped to small square)
            painter.drawPath(*path);

            // Check, pixel by pixel, to see if the curve
            // is in small rectangle around mouse click
            for ( int x = pt.x()-s/2; x < pt.x()+s/2; ++x ) {
                if ( x < 0 || x >= img.width() ) {
                    continue;
                }
                for ( int y = pt.y()-s/2; y < pt.y()+s/2; ++y ) {
                    if ( y < 0 || y >= img.height() ) {
                        continue;
                    }
                    QRgb pix = img.pixel(x,y);
                    if ( qRed(pix) == 0 ) {
                        // qRed(pix) is arbitrary. Pen is black and has no red.
                        isNear = true;
                        break;
                    }
                }
                if ( isNear ) break;
            }
        }

        delete path;
    }

    return isNear;
}

void CurvesView::mouseMoveEvent(QMouseEvent *event)
{
    QRectF W = viewport()->rect();
    if ( W.width() < 1 || W.height() < 1 ) return;

    double Ww = W.width();  // greater > 0, by top line
    double Wh = W.height();

    _mouseCurrPos = event->pos();

    if ( event->buttons() == Qt::NoButton && currentIndex().isValid() ) {

        QString tag = model()->data(currentIndex()).toString();
        QString presentation = _bookModel()->getDataString(rootIndex(),
                                                   "PlotPresentation","Plot");

        // If shift pressed while moving the mouse, do not update
        // the live coordinate.  This saves the live coord from
        // being updated while moving the mouse.
        Qt::KeyboardModifiers keymods = event->modifiers();
        bool shiftPressed = false;
        if ( keymods & Qt::ShiftModifier ) {
            shiftPressed = true;
        }

        QRectF M = _mathRect();
        double a = M.width()/W.width();
        double b = M.height()/W.height();
        double c = M.x() - a*W.x();
        double d = M.y() - b*W.y();
        QTransform T(a, 0,
                     0, b, /*+*/ c, d);
        QPointF wPt = event->pos();
        QPointF mPt = T.map(wPt);

        if ( !shiftPressed ) {

            CurveModel* curveModel =
                                    _bookModel()->getCurveModel(currentIndex());
            if ( curveModel ) {

                bool isDeletePath = false;
                QModelIndex plotIdx;
                QPainterPath* path = 0;
                if ( tag == "Curve" ) {
                    plotIdx = currentIndex().parent().parent();
                    path = _bookModel()->getPainterPath(currentIndex());
                } else if ( tag == "Plot" && presentation == "error" ) {
                    isDeletePath = true;
                    plotIdx = currentIndex();
                    QModelIndex curvesIdx = _bookModel()->getIndex(
                                                      plotIdx,"Curves", "Plot");
                    path = _bookModel()->getCurvesErrorPath(curvesIdx);
                } else {
                    fprintf(stderr, "koviz [bad scoobs]:1: "
                            "CurvesView::mouseMoveEvent\n");
                    exit(-1);
                }

                QModelIndex liveTimeIdx = _bookModel()->getDataIndex(
                                                                 QModelIndex(),
                                                               "LiveCoordTime");

                int rc = path->elementCount();

                QString plotXScale = _bookModel()->getDataString(plotIdx,
                                                           "PlotXScale","Plot");
                QString plotYScale = _bookModel()->getDataString(plotIdx,
                                                           "PlotYScale","Plot");
                double xs = 1.0;
                double ys = 1.0;
                double xb = 0.0;
                double yb = 0.0;
                QModelIndex idx = currentIndex();
                if ( plotXScale == "linear" && tag == "Curve" ) {
                    xb = _bookModel()->getDataDouble(idx,"CurveXBias","Curve");
                    xs = _bookModel()->getDataDouble(idx,"CurveXScale","Curve");
                }
                if ( plotYScale == "linear" && tag == "Curve" ) {
                    yb = _bookModel()->getDataDouble(idx,"CurveYBias","Curve");
                    ys = _bookModel()->getDataDouble(idx,"CurveYScale","Curve");
                }

                if ( curveModel->x()->name() == curveModel->t()->name() ) {

                    QPointF liveCoord(DBL_MAX,DBL_MAX);

                    if ( rc == 0 ) {

                        // "null" out liveCoord
                        liveCoord = QPointF(DBL_MAX,DBL_MAX);

                    } else if ( rc == 1 ) {

                        QPainterPath::Element el = path->elementAt(0);
                        liveCoord = QPointF(el.x*xs+xb,el.y*ys+yb);

                    } else if ( rc == 2 ) {
                        QPainterPath::Element el0 = path->elementAt(0);
                        QPainterPath::Element el1 = path->elementAt(1);
                        QPointF p0(el0.x*xs+xb,el0.y*ys+yb);
                        QPointF p1(el1.x*xs+xb,el1.y*ys+yb);
                        QLineF l0(p0,mPt);
                        QLineF l1(p1,mPt);
                        if ( l0.length() < l1.length() ) {
                            liveCoord = p0;
                        } else {
                            liveCoord = p1;
                        }

                    } else if ( rc >= 3 ) {

                        int i =  _idxAtTimeBinarySearch(path,0,rc-1,
                                                        (mPt.x()-xb)/xs);
                        QPainterPath::Element el = path->elementAt(i);
                        QPointF p(el.x*xs+xb,el.y*ys+yb);

                        //
                        // Make "neighborhood" around mouse point
                        //
                        QRectF M = _bookModel()->getPlotMathRect(rootIndex());
                        QRectF W = viewport()->rect();
                        double Wr = 2.0*fontMetrics().xHeight(); // near mouse
                        double Mr = Wr*(M.width()/W.width());

                        // Set j/k for finding min/maxs in next block of code
                        int j = i;
                        int k = i;
                        int nels = path->elementCount();
                        double iTime = path->elementAt(i).x*xs+xb;
                        double startTime = iTime - Mr;
                        double endTime = iTime + Mr;
                        for ( int l = i ; l >= 0; --l ) {
                            double lTime = path->elementAt(l).x*xs+xb;
                            if ( lTime > startTime ) {
                                j = l;
                            } else {
                                break;
                            }
                        }
                        for ( int l = i ; l < nels; ++l ) {
                            double lTime = path->elementAt(l).x*xs+xb;
                            if ( lTime < endTime ) {
                                k = l;
                            } else {
                                break;
                            }
                        }

                        //
                        // Find local min/maxs in neighborhood
                        //
                        QList<QPointF> localMaxs;
                        QList<QPointF> localMins;
                        QList<QPointF> flatChangePOIs;
                        for (int m = j; m <= k; ++m ) {
                            QPointF pt(path->elementAt(m).x*xs+xb,
                                       path->elementAt(m).y*ys+yb);
                            if ( m > 0 && m < k ) {
                                double yPrev = path->elementAt(m-1).y*ys+yb;
                                double y  = path->elementAt(m).y*ys+yb;
                                double yNext = path->elementAt(m+1).y*ys+yb;
                                if ( y > yPrev && y > yNext ) {
                                    if ( localMaxs.isEmpty() ) {
                                        localMaxs << pt;
                                    } else {
                                        if ( pt.y() > localMaxs.first().y() ) {
                                            localMaxs.prepend(pt);
                                        } else {
                                            localMaxs << pt;
                                        }
                                    }
                                } else if ( y < yPrev && y < yNext ) {
                                    if ( localMins.isEmpty() ) {
                                        localMins << pt;
                                    } else {
                                        if ( pt.y() < localMins.first().y() ) {
                                            localMins.prepend(pt);
                                        } else {
                                            localMins << pt;
                                        }
                                    }
                                } else if ( yPrev == y && y != yNext ) {
                                    if ( mPt.x() <= pt.x() ) {
                                        // mouse left of change
                                        flatChangePOIs.prepend(pt);
                                    }
                                } else if ( yPrev != y && y == yNext ) {
                                    if ( mPt.x() >= pt.x() ) {
                                        // mouse right of change
                                        flatChangePOIs.prepend(pt);
                                    }
                                }
                            }
                        }

                        //
                        // Choose live coord based on local mins/maxs
                        // and proximity to start/end points
                        //
                        if ( j == 0 || wPt.x()/W.width() < 0.02 ) {
                            // Mouse near curve start or left 2% of window,
                            // set to start pt
                            liveCoord = QPointF(path->elementAt(0).x*xs+xb,
                                                path->elementAt(0).y*ys+yb);
                        } else if ( k == rc-1 || wPt.x()/W.width() > 0.98 ) {
                            // Mouse near curve end or right 2% of window,
                            // set to last pt
                            liveCoord = QPointF(path->elementAt(k).x*xs+xb,
                                                path->elementAt(k).y*ys+yb);
                        } else {
                            bool isMaxs = localMaxs.isEmpty() ? false : true;
                            bool isMins = localMins.isEmpty() ? false : true;
                            if ( isMaxs && !isMins ) {
                                liveCoord = localMaxs.first();
                            } else if ( !isMaxs && isMins ) {
                                liveCoord = localMins.first();
                            } else if ( isMaxs && isMins ) {
                                // There are local mins and maxes
                                if ( wPt.y()/W.height() < 0.125 ) {
                                    // Mouse in top 1/8th of window
                                    liveCoord = localMaxs.first();
                                } else if ( mPt.y() > localMaxs.first().y() ) {
                                    // Mouse above curve
                                    liveCoord = localMaxs.first();
                                } else {
                                    // Mouse below curve
                                    liveCoord = localMins.first();
                                }
                            } else if ( !isMaxs && !isMins ) {
                                if ( !flatChangePOIs.isEmpty() ) {
                                    liveCoord = flatChangePOIs.first();
                                } else {
                                    liveCoord = p;
                                }
                            } else {
                                fprintf(stderr,"koviz [bad scoobs]:3: "
                                              "CurvesView::mouseMoveEvent()\n");
                                exit(-1);
                            }
                        }
                    }

                    if ( rc > 0 ) {
                        // Set live coord in model
                        double start =_bookModel()->getDataDouble(QModelIndex(),
                                                                   "StartTime");
                        double stop = _bookModel()->getDataDouble(QModelIndex(),
                                                                  "StopTime");
                        double time = liveCoord.x();
                        if ( plotXScale == "log" ) {
                            time = pow(10,time);
                        }
                        if ( time <= start ) {
                            time = start;
                        } else if ( time >= stop ) {
                            time = stop;
                        }
                        model()->setData(liveTimeIdx,time);

                        // If timestamps are identical, set liveTimeIndex so
                        // that livecoord is set to max y value
                        if ( plotXScale == "log") {
                            time = log10(time);
                        }
                        int i =  _idxAtTimeBinarySearch(path,0,
                                                        rc-1,(time-xb)/xs);
                        double iTime = path->elementAt(i).x;
                        int j = i;  // j is start index of identical timestamps
                        for ( int l = i; l >= 0; --l ) {
                            double lTime = path->elementAt(l).x;
                            if ( iTime != lTime ) {
                                break;
                            } else {
                                j = l;
                            }
                        }
                        int nels = path->elementCount();
                        int k = j; // k is last index of identical timestamps
                        for (int l = j; l < nels; ++l) {
                            double lTime = path->elementAt(l).x;
                            if ( iTime != lTime ) {
                                break;
                            } else {
                                k = l;
                            }
                        }
                        int liveCoordTimeIdx = 0;
                        if ( k - j > 1 ) {
                            // Find index of max y of identical timestamps
                            // Note: min is not used even if mouse below curve.
                            //       This is because I simply didn't want any
                            //       more code
                            double maxY = -DBL_MAX;
                            int m = 0 ;
                            for (int l = j; l <= k; ++l) {
                                double x = path->elementAt(l).x;
                                double y = path->elementAt(l).y;
                                if ( y > maxY ) {
                                    maxY = y;
                                    liveCoordTimeIdx = m;
                                }
                                if ( x != iTime ) {
                                    break;
                                }
                                ++m;
                            }
                        }
                        QModelIndex idx = _bookModel()->getDataIndex(
                                                       QModelIndex(),
                                                       "LiveCoordTimeIndex","");
                        _bookModel()->setData(idx,liveCoordTimeIdx);
                    }
                } else {  // Curve x is not time e.g. ball xy-position

                    double start = _bookModel()->getDataDouble(QModelIndex(),
                                                             "StartTime");
                    double stop = _bookModel()->getDataDouble(QModelIndex(),
                                                            "StopTime");
                    double xb = _bookModel()->getDataDouble(currentIndex(),
                                                          "CurveXBias","Curve");
                    double xs = _bookModel()->getDataDouble(currentIndex(),
                                                         "CurveXScale","Curve");
                    double yb = _bookModel()->getDataDouble(currentIndex(),
                                                          "CurveYBias","Curve");
                    double ys = _bookModel()->getDataDouble(currentIndex(),
                                                         "CurveYScale","Curve");
                    double xus = 1.0;
                    double xub = 0.0;
                    QString bookXUnit = _bookModel()->getDataString(
                                                          currentIndex(),
                                                          "CurveXUnit","Curve");
                    if ( !bookXUnit.isEmpty() && bookXUnit != "--" ) {
                        QString loggedXUnit = curveModel->x()->unit();
                        xus = Unit::scale(loggedXUnit, bookXUnit);
                        xub = Unit::bias(loggedXUnit, bookXUnit);
                    }
                    double yus = 1.0;
                    double yub = 0.0;
                    QString bookYUnit = _bookModel()->getDataString(
                                                          currentIndex(),
                                                          "CurveYUnit","Curve");
                    if ( !bookYUnit.isEmpty() && bookYUnit != "--" ) {
                        QString loggedYUnit = curveModel->y()->unit();
                        yus = Unit::scale(loggedYUnit, bookYUnit);
                        yub = Unit::bias(loggedYUnit, bookYUnit);
                    }
                    bool isXLogScale = (plotXScale=="log") ? true : false;
                    bool isYLogScale = (plotYScale=="log") ? true : false;
                    double liveTime = DBL_MAX;
                    double dMin = DBL_MAX;
                    QTransform T = _coordToPixelTransform();
                    curveModel->map();
                    ModelIterator* it = curveModel->begin();
                    while ( !it->isDone() ) {
                        // find closest point on curve to mouse
                        double x = it->x();
                        x = xus*x + xub;
                        x = xs*x + xb;
                        if ( isXLogScale ) {
                            if ( x > 0 ) {
                                x = log10(x);
                            } else if ( x < 0 ) {
                                x = log10(-x);
                            } else if ( x == 0 ) {
                                it->next();
                                continue; // skip since log(0) -inf
                            } else {
                                // bad scoobs
                                it->next();
                                continue;
                            }
                        }
                        double y = it->y();
                        y = yus*y + yub;
                        y = ys*y + yb;
                        if ( isYLogScale ) {
                            if ( y > 0 ) {
                                y = log10(y);
                            } else if ( y < 0 ) {
                                y = log10(-y);
                            } else if ( y == 0 ) {
                                it->next();
                                continue; // skip since log(0) -inf
                            } else {
                                // bad scoobs
                                it->next();
                                continue;
                            }
                        }
                        QPointF p(x,y);
                        QPointF q = T.map(p);
                        double d = QLineF(wPt,q).length();
                        if ( d < dMin && start <= it->t() && it->t() <= stop ) {
                            dMin = d;
                            liveTime = it->t();
                        }
                        it->next();
                    }
                    delete it;
                    curveModel->unmap();

                    // Set live coord in model
                    if ( liveTime <= start ) {
                        model()->setData(liveTimeIdx,start);
                    } else if ( liveTime >= stop ) {
                        model()->setData(liveTimeIdx,stop);
                    } else {
                        model()->setData(liveTimeIdx,liveTime);
                    }
                }

                viewport()->update();

                if ( isDeletePath ) {
                    delete path;
                }
            }
        }
    } else if ( event->buttons() == _buttonSelectAndPan ) {

        Qt::KeyboardModifiers keymods = event->modifiers();
        bool isAltKey = false;
        if ( keymods & Qt::AltModifier ) {
            isAltKey = true;
        }
        bool isCtrlKey = false;
        if ( keymods & Qt::ControlModifier ) {
            isCtrlKey = true;
        }

        if ( isAltKey && !isCtrlKey ) {
            // Measuring line with dx,dy
            _isMeasure = true; // Tell paint event to draw line
            QRectF M = _bookModel()->getPlotMathRect(rootIndex());
            QRectF W = viewport()->rect();
            QString xScale = _bookModel()->getDataString(rootIndex(),
                                                         "PlotXScale", "Plot");
            QString yScale = _bookModel()->getDataString(rootIndex(),
                                                         "PlotYScale", "Plot");
            double M_x_pres = M.left()+_mousePressPos.x()*(M.width()/W.width());
            double M_x_curr = M.left()+ _mouseCurrPos.x()*(M.width()/W.width());
            if ( xScale == "log" ) {
                M_x_pres = pow(10,M_x_pres);
                M_x_curr = pow(10,M_x_curr);
            }
            double dx = qAbs(M_x_curr-M_x_pres);

            double M_y_pres = M.top()-
                              _mousePressPos.y()*qAbs(M.height()/W.height());
            double M_y_curr = M.top()-
                               _mouseCurrPos.y()*qAbs(M.height()/W.height());
            if ( yScale == "log" ) {
                M_y_pres = pow(10,M_y_pres);
                M_y_curr = pow(10,M_y_curr);
            }
            double dy = qAbs(M_y_curr-M_y_pres);

            QModelIndex curvesIdx = _bookModel()->getIndex(rootIndex(),
                                                           "Curves","Plot");
            QString xUnit = _bookModel()->getCurvesXUnit(curvesIdx);
            QString yUnit = _bookModel()->getCurvesYUnit(curvesIdx);
            QModelIndex statusIdx = _bookModel()->getDataIndex(QModelIndex(),
                                                         "StatusBarMessage","");
             QString msg = QString("dx=%1 {%2} dy=%3 {%4}")
                                  .arg(dx).arg(xUnit).arg(dy).arg(yUnit);
             _bookModel()->setData(statusIdx,msg);
             viewport()->update();

        } else if ( isCtrlKey == true && currentIndex().isValid() ) {
            // Drag curve (snap to init point of another curve with ctrl+alt)
            QString tag = model()->data(currentIndex()).toString();
            QString presentation = _bookModel()->getDataString(rootIndex(),
                                                     "PlotPresentation","Plot");
            QString xScale = _bookModel()->getDataString(rootIndex(),
                                                         "PlotXScale","Plot");
            QString yScale = _bookModel()->getDataString(rootIndex(),
                                                         "PlotYScale","Plot");
            bool isScaleLinear = false;
            if ( xScale == "linear" && yScale == "linear" ) {
                isScaleLinear = true;
            } else {
                QMessageBox msgBox;
                QString msg = QString("Curve dragging only "
                                      "works in linear scale");
                msgBox.setText(msg);
                msgBox.exec();
            }
            if ( (tag == "Curve" && isScaleLinear &&
                 (presentation == "compare" || presentation.isEmpty())) ) {

                QModelIndex curveIdx = currentIndex();
                CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);

                if ( curveModel ) {
                    QTransform M2W = _coordToPixelTransform();
                    double xInitWin = 0.0;
                    double yInitWin = 0.0;
                    double xInit = 0.0;
                    double yInit = 0.0;
                    double xInitBias = 0.0;
                    double yInitBias = 0.0;
                    curveModel->map();
                    ModelIterator* it = curveModel->begin();
                    it->start();
                    if ( ! it->isDone() ) {
                        double xs = _bookModel()->getDataDouble(
                                                curveIdx,"CurveXScale","Curve");
                        double xb = _bookModel()->getDataDouble(
                                                curveIdx,"CurveXBias","Curve");
                        double ys = _bookModel()->getDataDouble(
                                                curveIdx,"CurveYScale","Curve");
                        double yb = _bookModel()->getDataDouble(
                                                curveIdx,"CurveYBias","Curve");
                        xInitBias = xb;
                        yInitBias = yb;
                        xInit = it->at(0)->x()*xs + xb;
                        yInit = it->at(0)->y()*ys + yb;
                        QPointF p(xInit,yInit);
                        p = M2W.map(p);
                        xInitWin = p.x();
                        yInitWin = p.y();
                    }
                    curveModel->unmap();
                    delete it;

                    bool isSnap = false;
                    double xSnapBias = 0.0;
                    double ySnapBias = 0.0;
                    if ( isAltKey ) {
                        QModelIndex curvesIdx = curveIdx.parent();
                        QModelIndexList curveIdxs = _bookModel()->
                                      getIndexList(curvesIdx, "Curve","Curves");
                        foreach ( QModelIndex crvIdx, curveIdxs ) {
                            if ( crvIdx == currentIndex() ) {
                                continue;
                            }
                            CurveModel* curve = _bookModel()->
                                    getCurveModel(crvIdx);
                            if ( curve ) {
                                curve->map();
                                ModelIterator* it = curve->begin();
                                it->start();
                                if ( ! it->isDone() ) {
                                    double xs = _bookModel()->getDataDouble(
                                                  crvIdx,"CurveXScale","Curve");
                                    double xb = _bookModel()->getDataDouble(
                                                   crvIdx,"CurveXBias","Curve");
                                    double ys = _bookModel()->getDataDouble(
                                                  crvIdx,"CurveYScale","Curve");
                                    double yb = _bookModel()->getDataDouble(
                                                   crvIdx,"CurveYBias","Curve");
                                    double x0 = it->at(0)->x()*xs + xb;
                                    double y0 = it->at(0)->y()*ys + yb;
                                    QPointF p(x0,y0);
                                    p = M2W.map(p);
                                    double xWin = p.x();
                                    double yWin = p.y();

                                    double dxw = xInitWin-xWin;
                                    double dyw = yInitWin-yWin;
                                    double d = qSqrt(dxw*dxw + dyw*dyw);

                                    if ( d < 30 ) {
                                        isSnap = true;
                                        double dx = xInit-x0;
                                        double dy = yInit-y0;
                                        xSnapBias = xInitBias-dx;
                                        ySnapBias = yInitBias-dy;
                                        break;
                                    }

                                }
                                delete it;
                                curve->unmap();
                            }
                        }
                    }

                    QModelIndex xBiasIdx = _bookModel()->getDataIndex(
                                                 curveIdx,"CurveXBias","Curve");
                    QModelIndex yBiasIdx = _bookModel()->getDataIndex(
                                                 curveIdx,"CurveYBias","Curve");
                    if ( isSnap ) {
                        _bookModel()->setData(xBiasIdx, xSnapBias);
                        _bookModel()->setData(yBiasIdx, ySnapBias);
                    } else {
                        QRectF M = _mathRect();
                        double Mw = M.width();
                        double Mh = M.height();
                        QTransform W2M(Mw/Ww, 0.0, // div by zero checked at top
                                       0.0, Mh/Wh,
                                       0.0, 0.0);
                        QPointF dW = event->pos()-_mousePressPos;
                        QPointF dM = W2M.map(dW);
                        double xbias = _mousePressXBias+dM.x();
                        double ybias = _mousePressYBias+dM.y();
                        _bookModel()->setData(xBiasIdx, xbias);
                        _bookModel()->setData(yBiasIdx, ybias);
                    }
                }
            }
        } else {
            // Pan or scale
            double k = 0.88;
            QRectF insideRect((1-k)*Ww/2.0,(1-k)*Wh/2.0,k*Ww,k*Wh);

            if ( insideRect.contains(_mousePressPos) ) {
                // Pan if mouse press pos is deeper inside window
                QRectF M = _mathRect();
                double Mw = M.width();
                double Mh = M.height();
                QTransform T(Mw/Ww, 0.0,  // div by zero checked at top of method
                             0.0, Mh/Wh,
                             0.0, 0.0);
                QPointF dW = event->pos()-_mousePressPos;
                QPointF mPt = _mousePressMathRect.topLeft()-T.map(dW);
                M.moveTo(mPt);
                _bookModel()->setPlotMathRect(M,rootIndex());
                viewport()->update();
            } else {
                // Scale if mouse press pos is on perifery of window
                QRectF leftRect(0,0,
                                (1-k)*Ww/2.0, Wh);
                QRectF rightRect(insideRect.topRight().x(),0,
                                 (1-k)*Ww/2.0,Wh);
                QRectF topRect(insideRect.topLeft().x(), 0,
                               insideRect.width(),(1-k)*Wh/2.0);
                QRectF botRect(insideRect.bottomLeft().x(),
                               insideRect.bottomLeft().y(),
                               insideRect.width(),(1-k)*Wh/2.0);

                if ( rightRect.contains(_mousePressPos) ) {
                    _alignment = Qt::AlignRight;
                } else if ( leftRect.contains(_mousePressPos) ) {
                    _alignment = Qt::AlignLeft;
                } else if ( topRect.contains(_mousePressPos) ) {
                    _alignment = Qt::AlignTop;
                } else if ( botRect.contains(_mousePressPos) ) {
                    _alignment = Qt::AlignBottom;
                } else {
                    // shouldn't happen, but ignore in any case
                }

                BookIdxView::mouseMoveEvent(event); // Scale logic here
            }
        }
    } else {
        event->ignore();
    }
}

void CurvesView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Period: _keyPressPeriod();break;
    case Qt::Key_Space: _keyPressSpace();break;
    case Qt::Key_Up: _keyPressUp();break;
    case Qt::Key_Down: _keyPressDown();break;
    case Qt::Key_Left: _keyPressArrow(Qt::LeftArrow);break;
    case Qt::Key_Right: _keyPressArrow(Qt::RightArrow);break;
    case Qt::Key_Comma: _keyPressComma();break;
    case Qt::Key_Escape: _keyPressEscape();break;
    case Qt::Key_F: _keyPressF();break;
    case Qt::Key_B: _keyPressB();break;
    case Qt::Key_G: _keyPressG();break;
    case Qt::Key_D: _keyPressD();break;
    case Qt::Key_I: _keyPressI();break;
    case Qt::Key_S: _keyPressS();break;
    case Qt::Key_M: _keyPressM();break;
    case Qt::Key_E: _keyPressE();break;
    case Qt::Key_Minus: _keyPressMinus();break;
    default: ; // do nothing
    }
}

void CurvesView::currentChanged(const QModelIndex &current,
                                const QModelIndex &previous)
{
    QModelIndex statusIdx = _bookModel()->getDataIndex(QModelIndex(),
                                                       "StatusBarMessage","");
    QString tag = _bookModel()->data(current).toString();
    if ( tag == "Curve" && this->hasFocus() ) {
        QString yName = _bookModel()->getDataString(current,
                                                    "CurveYName","Curve");
        QString yUnit = _bookModel()->getDataString(current,
                                                    "CurveYUnit","Curve");
        CurveModel* curveModel = _bookModel()->getCurveModel(current);
        QString run;
        if ( curveModel ) {
            run = curveModel->runPath();
        }
        QString msg = yName + " {" + yUnit + "} " + run ;
        _bookModel()->setData(statusIdx,msg); // PlotMainWindow uses this
    } else if ( !current.isValid() && previous.isValid() ) {
        // Clicked off of curve into whitespace
        QString msg = "";
        _bookModel()->setData(statusIdx,msg);

        // Set live time to empty
        QModelIndex liveIdx = _bookModel()->getDataIndex(QModelIndex(),
                                                         "LiveCoordTime");
        if ( !_bookModel()->data(liveIdx).toString().isEmpty() ) {
            _bookModel()->setData(liveIdx, "");
        }
    }
    viewport()->update();
}

void CurvesView::resizeEvent(QResizeEvent *event)
{
    if ( _pixmap ) {
        delete _pixmap;
    }
    _pixmap = _createLivePixmap();

    QAbstractItemView::resizeEvent(event);
}

// For two curves hitting the spacebar will toggle between viewing
// the two curves in error, compare and error+compare views
void CurvesView::_keyPressSpace()
{
    QModelIndex curvesIdx = _bookModel()->getIndex(rootIndex(),"Curves","Plot");
    int rc = model()->rowCount(curvesIdx);
    if ( rc != 2 ) return;

    QString plotPresentation = _bookModel()->getDataString(rootIndex(),
                                                   "PlotPresentation","Plot");

    if ( plotPresentation == "error" || plotPresentation.isEmpty() ) {
        plotPresentation = "compare";
    } else if ( plotPresentation == "compare" ) {
        plotPresentation = "error+compare";
    } else if ( plotPresentation == "error+compare" ) {
        plotPresentation = "error";
    } else {
        fprintf(stderr,"koviz [bad scoobs]: _keyPressSpace() : "
                       "plotPresentation=\"%s\"\n",
                       plotPresentation.toLatin1().constData());
        exit(-1);
    }

    // Make units the same (in the book model)
    QModelIndex idx0 = model()->index(0,0,curvesIdx);
    QModelIndex idx1 = model()->index(1,0,curvesIdx);
    QString dpYUnit0 = _bookModel()->getDataString(idx0,"CurveYUnit","Curve");
    QString dpYUnit1 = _bookModel()->getDataString(idx1,"CurveYUnit","Curve");
    CurveModel* c0 = _bookModel()->getCurveModel(curvesIdx,0);
    if ( c0->y()->unit() != dpYUnit0 ) {
        if ( dpYUnit0.isEmpty() || dpYUnit0 == "--" ) {
            QModelIndex unitIdx0 = _bookModel()->getDataIndex(idx0,
                                                         "CurveYUnit", "Curve");
            model()->setData(unitIdx0,c0->y()->unit());
            dpYUnit0 = c0->y()->unit();
        }
    }
    if ( dpYUnit0 != dpYUnit1 && !dpYUnit0.isEmpty() && !dpYUnit1.isEmpty() ) {
        // Make dp units the same (if in same family)
        QString u0(dpYUnit0);
        QString u1(dpYUnit1);
        if ( Unit::canConvert(u0,u1) ) {
            QModelIndex unitIdx1 = _bookModel()->getDataIndex(idx1,
                                                         "CurveYUnit", "Curve");
            model()->setData(unitIdx1,dpYUnit0);
        }
    }

    // Set presentation
    QModelIndex plotPresentationIdx = _bookModel()->getDataIndex(rootIndex(),
                                                   "PlotPresentation","Plot");
    model()->setData(plotPresentationIdx,plotPresentation);

    // Recalculate plot math rect to contain curves in toggled presentation
    QRectF bbox = _bookModel()->calcCurvesBBox(curvesIdx);
    QRectF plotMathRect = _bookModel()->getPlotMathRect(rootIndex());
    double x0 = plotMathRect.left();
    double y0 = bbox.top();
    double w = plotMathRect.width();
    double h = bbox.height();
    QRectF R(x0,y0,w,h);
    _bookModel()->setPlotMathRect(R,rootIndex());

    // Null out the current index
    setCurrentIndex(QModelIndex());

    viewport()->update();
}

void CurvesView::_keyPressUp()
{
    if ( _bookModel()->isIndex(currentIndex(),"Curve") ) {
        QModelIndex curveIdx = currentIndex();
        QModelIndex curvesIdx = curveIdx.parent();
        int i = curveIdx.row();
        if ( i > 0 ) {
            QModelIndex nextCurveIdx = model()->index(i-1,0,curvesIdx);
            setCurrentIndex(nextCurveIdx);
        }
    }
}

void CurvesView::_keyPressDown()
{
    if ( _bookModel()->isIndex(currentIndex(),"Curve") ) {
        QModelIndex curveIdx = currentIndex();
        QModelIndex curvesIdx = curveIdx.parent();
        int rc = model()->rowCount(curvesIdx);
        int i = curveIdx.row();
        if ( i+1 < rc ) {
            QModelIndex nextCurveIdx = model()->index(i+1,0,curvesIdx);
            setCurrentIndex(nextCurveIdx);
        }
    }
}

void CurvesView::_keyPressComma()
{
    if ( !currentIndex().isValid() ) return;

    if ( currentIndex().column() != 0 ) return;

    QString tag = _bookModel()->data(currentIndex()).toString();
    if ( tag != "Curve" && tag != "Plot" ) {
        return;
    }

    // Get live time
    QModelIndex liveIdx = _bookModel()->getDataIndex(QModelIndex(),
                                                     "LiveCoordTime");
    double liveTime = model()->data(liveIdx).toDouble();

    QModelIndex liveTimeIdxIdx = _bookModel()->getDataIndex(QModelIndex(),
                                                     "LiveCoordTimeIndex");
    int timeIdx = model()->data(liveTimeIdxIdx).toInt();

    // Add Markers parent to either Plot or Curve
    QModelIndex idx = currentIndex();
    QStandardItem *currItem = _bookModel()->itemFromIndex(idx);
    if ( tag == "Curve" ) {
        if (!_bookModel()->isChildIndex(idx,"Curve","Markers")) {
            _bookModel()->addChild(currItem, "Markers","");
        }
    } else if ( tag == "Plot" ) {
        if (!_bookModel()->isChildIndex(idx,"Plot","Markers")) {
            _bookModel()->addChild(currItem, "Markers","");
        }
    }
    QModelIndex markersIdx = _bookModel()->getIndex(idx,"Markers","");

    // Add/remove marker to/from Bookmodel
    bool isRemoved = false;
    QModelIndexList markerIdxs = _bookModel()->getIndexList(markersIdx,
                                                            "Marker","Markers");
    foreach ( QModelIndex markerIdx, markerIdxs ) {
        double markerTime = _bookModel()->getDataDouble(markerIdx,
                                                        "MarkerTime","Marker");
        int markerTimeIdx = _bookModel()->getDataInt(markerIdx,
                                                     "MarkerTimeIdx","Marker");
        if ( liveTime == markerTime && timeIdx == markerTimeIdx ) {
            _bookModel()->removeRow(markerIdx.row(),markersIdx);
            isRemoved = true;
            break;
        }
    }
    if ( !isRemoved ) {
        QStandardItem* markersItem =_bookModel()->itemFromIndex(markersIdx);
        QStandardItem* markerItem = _bookModel()->addChild(markersItem,
                                                           "Marker","");
        _bookModel()->addChild(markerItem,"MarkerTime", liveTime);
        _bookModel()->addChild(markerItem,"MarkerTimeIdx", timeIdx);
        _bookModel()->addChild(markerItem,"MarkerLabel", "");
    }
}

void CurvesView::_keyPressEscape()
{
    QModelIndex curvesIdx = _bookModel()->getIndex(rootIndex(),
                                                   "Curves","Plot");
    QRectF bbox = _bookModel()->calcCurvesBBox(curvesIdx);
    _bookModel()->setPlotMathRect(bbox,rootIndex());
    viewport()->update();
}

// Toggle between Time and Frequency domain
void CurvesView::_keyPressF()
{
    QModelIndex plotIdx = rootIndex();
    QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
    QModelIndexList curveIdxs = _bookModel()->getIndexList(curvesIdx,
                                                           "Curve","Curves");

    // All curves should be time or frequency domain (x should be "s" or "Hz")
    bool isTimeDomain = false;
    bool isFrequencyDomain = false;
    QString xUnit = _bookModel()->getCurvesXUnit(curvesIdx);
    if ( xUnit == "s" ) {
        isTimeDomain = true;
    } else if ( xUnit == "Hz" ) {
        isFrequencyDomain = true;
    }
    if ( !isTimeDomain && !isFrequencyDomain ) {
        QMessageBox msgBox;
        QString msg = QString("Attempting to take Fourier transform on data "
                              "where x units are neither seconds or Hz. "
                              "Aborting!");
        msgBox.setText(msg);
        msgBox.exec();
        return;
    }
    if ( isFrequencyDomain && !_fftCache.isCache ) {
        QMessageBox msgBox;
        QString msg = QString("Attempting to take Inverse Fourier transform "
                              "on data logged in Hz.  Sorry, this is not "
                              "implemented. Aborting!");
        msgBox.setText(msg);
        msgBox.exec();
        return;
    }

    QRectF M = _bookModel()->getPlotMathRect(rootIndex());

    QModelIndex xAxisLabelIdx = _bookModel()->getDataIndex(plotIdx,
                                                       "PlotXAxisLabel","Plot");
    QModelIndex startTimeIdx = _bookModel()->getDataIndex(QModelIndex(),
                                                          "StartTime");
    QModelIndex stopTimeIdx = _bookModel()->getDataIndex(QModelIndex(),
                                                         "StopTime");

    QProgressDialog progress("Time/Frequency domain", "Abort", 0,
                             curveIdxs.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(500);
    int i = 0;

    if ( isTimeDomain ) {
        // TimeDomain -> FrequencyDomain
        _fftCache.isCache = true;
        _fftCache.xAxisLabel = _bookModel()->data(xAxisLabelIdx).toString();
        _fftCache.M = M;
        _fftCache.start = _bookModel()->data(startTimeIdx).toDouble();
        _fftCache.stop = _bookModel()->data(stopTimeIdx).toDouble();
        _fftCache.curveCaches.clear();
        bool block = _bookModel()->blockSignals(true);
        foreach ( QModelIndex curveIdx, curveIdxs ) {
            CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
            double xb = _bookModel()->getDataDouble(curveIdx,
                                                    "CurveXBias","Curve");
            double xs = _bookModel()->getDataDouble(curveIdx,
                                                    "CurveXScale","Curve");
            FFTCurveCache* cache = new FFTCurveCache(xb,xs,curveModel);
            _fftCache.curveCaches.append(cache);

            CurveModel* fft = new CurveModelFFT(curveModel,
                                                xb,xs,
                                                M.left(),M.right());
            QVariant v = PtrToQVariant<CurveModel>::convert(fft);
            QModelIndex curveDataIdx = _bookModel()->getDataIndex(curveIdx,
                                                           "CurveData","Curve");
            QModelIndex xUnitIdx = _bookModel()->getDataIndex(curveIdx,
                                                          "CurveXUnit","Curve");
            QModelIndex xBiasIdx = _bookModel()->getDataIndex(curveIdx,
                                                          "CurveXBias","Curve");
            QModelIndex xScaleIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveXScale","Curve");
            _bookModel()->setData(xUnitIdx,"Hz");
            _bookModel()->setData(xBiasIdx,0.0);
            _bookModel()->setData(xScaleIdx,1.0);
            _bookModel()->setData(curveDataIdx,v);

            progress.setValue(i++);
            if (progress.wasCanceled()) {
                break;
            }
            QString msg = QString("Loaded %1 of %2 curves")
                             .arg(i).arg(curveIdxs.size());
            progress.setLabelText(msg);
        }
        QRectF bbox = _bookModel()->calcCurvesBBox(curvesIdx);
        _bookModel()->setPlotMathRect(bbox,rootIndex());
        _bookModel()->setData(startTimeIdx,-DBL_MAX);
        _bookModel()->setData(stopTimeIdx,DBL_MAX);
        _bookModel()->setData(xAxisLabelIdx,"Frequency");
        _bookModel()->blockSignals(block);
        QModelIndex xScaleIdx = _bookModel()->getDataIndex(plotIdx,
                                                           "PlotXScale","Plot");
        _bookModel()->setData(xScaleIdx,"log");
    } else {
        // FrequencyDomain -> TimeDomain
        _bookModel()->setData(xAxisLabelIdx,_fftCache.xAxisLabel);
        _bookModel()->setPlotMathRect(_fftCache.M,plotIdx);
        _bookModel()->setData(startTimeIdx,_fftCache.start);
        _bookModel()->setData(stopTimeIdx,_fftCache.stop);
        bool block = _bookModel()->blockSignals(true);
        foreach ( QModelIndex curveIdx, curveIdxs ) {
            QModelIndex xUnitIdx = _bookModel()->getDataIndex(curveIdx,
                                                          "CurveXUnit","Curve");
            _bookModel()->setData(xUnitIdx,"s");
            QModelIndex xBiasIdx = _bookModel()->getDataIndex(curveIdx,
                                                          "CurveXBias","Curve");
            QModelIndex xScaleIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveXScale","Curve");
            QModelIndex curveDataIdx = _bookModel()->getDataIndex(curveIdx,
                                                           "CurveData","Curve");
            FFTCurveCache* cache = _fftCache.curveCaches.takeFirst();

            _bookModel()->setData(xBiasIdx,cache->xbias());
            _bookModel()->setData(xScaleIdx,cache->xscale());

            CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
            if ( curveModel ) {
                delete curveModel;
            }
            QVariant v=PtrToQVariant<CurveModel>::convert(cache->curveModel());
            _bookModel()->setData(curveDataIdx,v);

            progress.setValue(i++);
            if (progress.wasCanceled()) {
                break;
            }
            QString msg = QString("Loaded %1 of %2 curves")
                             .arg(i).arg(curveIdxs.size());
            progress.setLabelText(msg);
        }
        QModelIndex xScaleIdx = _bookModel()->getDataIndex(plotIdx,
                                                           "PlotXScale","Plot");
        _bookModel()->setData(xScaleIdx,"linear");
        _bookModel()->blockSignals(block);
        _bookModel()->setPlotMathRect(_fftCache.M,plotIdx);
    }

    progress.setValue(curveIdxs.size());
}

void CurvesView::_keyPressB()
{
    QModelIndex plotIdx = rootIndex();
    QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
    QModelIndexList curveIdxs = _bookModel()->getIndexList(curvesIdx,
                                                           "Curve","Curves");

    foreach ( QModelIndex curveIdx, curveIdxs ) {
        CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
        if ( curveModel->x()->unit() != "s" ) {
            QMessageBox msgBox;
            QString msg = QString("Butterworth filter expects xunits to be "
                                  "seconds.  Please try again.\n");
            msgBox.setText(msg);
            msgBox.exec();
            return;
        }
    }

    if ( ! _bw_frame ) {
        _bw_frame = new QFrame(this);
        _bw_slider = new QSlider(_bw_frame);
        _bw_label = new QLineEdit(_bw_frame);

        // Shrink text box since default stretches over plot
        QFontMetrics fm(_bw_label->font());
        int w = fm.averageCharWidth()*7; // 7 is an arbitrary init value
        _bw_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        _bw_label->setMinimumWidth(w);

        _bw_label->setToolTip("Frequency");

        // Validate label entry
        QIntValidator* v = new QIntValidator(this);
        _bw_label->setValidator(v);

    } else if ( _bw_frame->isHidden() ) {
        _bw_frame->show();
        return;
    } else {
        _bw_frame->hide();
        return;
    }
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(_bw_label);
    layout->addWidget(_bw_slider);
    _bw_frame->setLayout(layout);

    connect(_bw_slider,SIGNAL(valueChanged(int)),
            this,SLOT(_keyPressBSliderChanged(int)));
    connect(_bw_label,SIGNAL(returnPressed()),
            this,SLOT(_keyPressBLineEditReturnPressed()));

    // Get Nyquist frequency for bw filter
    double dtMin = DBL_MAX;
    foreach ( QModelIndex curveIdx, curveIdxs ) {
        CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
        if ( curveModel ) {
            curveModel->map();
            int N = 0;
            double dt = 0.0;
            bool isFirst = true;
            double lastTime = 0.0;
            double lastDt = 0.0;
            ModelIterator* it = curveModel->begin();
            while ( !it->isDone() ) {
                if ( isFirst ) {
                    isFirst = false;
                } else {
                    dt = it->t() - lastTime;
                    if (lastDt > 0.0 && dt > 0.0 && qAbs(dt-lastDt) > 1.0e-9) {
                        QMessageBox msgBox;
                        QString msg = QString(
                                    "Butterworth filter expects "
                                    "uniform sampling frequency.  "
                                    "Data has variable dt.  Bailing!");
                        msgBox.setText(msg);
                        msgBox.exec();
                        delete it;
                        curveModel->unmap();
                        _bw_frame->hide();
                        return;
                    } else if ( dt == 0.0 ) {
                        QMessageBox msgBox;
                        QString msg = QString(
                                   "Butterworth filter expects "
                                   "uniform sampling frequency.  "
                                   "Data has two values with same time stamp.  "
                                   "Bailing!");
                        msgBox.setText(msg);
                        msgBox.exec();
                        delete it;
                        curveModel->unmap();
                        _bw_frame->hide();
                        return;
                    }
                    lastDt = dt;
                }
                lastTime = it->t();
                ++N;
                it->next();
            }
            if ( dt > 0 && dt < dtMin ) {
                dtMin = dt;
            }

            double minSamplingRate = 4.0; // Hz
            if ( dtMin > 1/minSamplingRate ) {
                QMessageBox msgBox;
                QString msg = QString(
                            "Attempted filter of data with a low sampling rate "
                            "of %1Hz.  "
                            "Butterworth filter expects "
                            "a sampling frequency greater than %2Hz.  "
                            "Bailing!").arg(1/dtMin).arg(minSamplingRate);
                msgBox.setText(msg);
                msgBox.exec();
                _bw_frame->hide();
                return;
            }

            // Cache off original data for later filtering (use _real as cache)
            curveModel->_real = (double*)malloc(N*sizeof(double));

            it = it->at(0);
            double goodVal = 0.0;
            while ( !it->isDone() ) {
                if ( std::isnan(it->y()) ) {
                    it->next();
                    continue;
                }
                goodVal = it->y();
                break;
            }

            int i = 0;
            it = it->at(0);
            while ( !it->isDone() ) {
                curveModel->_real[i] = it->y();
                if ( std::isnan(curveModel->_real[i]) ) {
                    curveModel->_real[i] = goodVal;
                }
                goodVal = curveModel->_real[i];
                it->next();
                ++i;
            }
        }
    }
    int maxFilterFreq = 0; // Nyquist frequency - 1
    if ( qRound(1.0/dtMin) % 2 == 0 ) {
        // even
        maxFilterFreq = qRound(0.5*(1.0/dtMin))-1;
    } else {
        // odd
        maxFilterFreq = qFloor(0.5*(1.0/dtMin));
    }
    _bw_slider->setRange(1,maxFilterFreq);
    QIntValidator* v = (QIntValidator*)_bw_label->validator();
    v->setRange(1,maxFilterFreq);
    if ( maxFilterFreq > 7 ) {
        _bw_slider->setValue(7); // Setting to 7Hz just because it works a lot
    } else {
        _bw_slider->setValue(1);
    }

    _bw_slider->show();
    _bw_label->show();
    _bw_frame->show();
}

void CurvesView::_keyPressBSliderChanged(int value)
{
    QModelIndex plotIdx = rootIndex();
    QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
    QModelIndexList curveIdxs = _bookModel()->getIndexList(curvesIdx,
                                                           "Curve","Curves");

    bool block = _bookModel()->blockSignals(true);
    foreach ( QModelIndex curveIdx, curveIdxs ) {
        CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
        if ( curveModel ) {
            CurveModel* bw = new CurveModelBW(curveModel,value);
            QVariant v = PtrToQVariant<CurveModel>::convert(bw);
            QModelIndex curveDataIdx = _bookModel()->getDataIndex(curveIdx,
                                                           "CurveData","Curve");
            _bookModel()->setData(curveDataIdx,v);
            curveModel->_real = 0; // Zero out cache since bw now owns it
            delete curveModel;
        }
    }
    _bookModel()->blockSignals(block);

    // Reset bounding box so that plot refreshes (optimizing redraw)
    QRectF Z; // Empty
    QRectF M = _bookModel()->getPlotMathRect(plotIdx);
    _bookModel()->setPlotMathRect(Z,plotIdx);
    _bookModel()->setPlotMathRect(M,plotIdx);

    // Update lineedit label
    QString s = QString("%1").arg(value);
    _bw_label->setText(s);
}

void CurvesView::_keyPressBLineEditReturnPressed()
{
    QString s = _bw_label->text();
    bool ok;
    int value = s.toInt(&ok);
    if ( ok ) {
        _keyPressBSliderChanged(value);

        // Update slider
        _bw_slider->setValue(value);
    }
}

void CurvesView::_keyPressG()
{
    QModelIndex plotIdx = rootIndex();
    QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
    QModelIndexList curveIdxs = _bookModel()->getIndexList(curvesIdx,
                                                           "Curve","Curves");
    foreach ( QModelIndex curveIdx, curveIdxs ) {
        CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
        if ( curveModel->x()->unit() != "s" ) {
            QMessageBox msgBox;
            QString msg = QString("S-Golay filter expects xunits to be "
                                  "seconds.  Please try again.\n");
            msgBox.setText(msg);
            msgBox.exec();
            return;
        }
    }

    if ( ! _sg_frame ) {
        _sg_frame = new QFrame(this);
        _sg_slider = new QSlider(_sg_frame);
        _sg_window = new QLineEdit(_sg_frame);
        _sg_degree = new QLineEdit(_sg_frame);

        // Shrink text box since default stretches over plot
        QFontMetrics fm(_sg_window->font());
        int w = fm.averageCharWidth()*7; // 7 is an arbitrary init value
        _sg_window->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        _sg_window->setMinimumWidth(w);
        _sg_window->setToolTip("SG Window");
        _sg_degree->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        _sg_degree->setMinimumWidth(w);
        _sg_degree->setToolTip("Degree: 2-6");

        // Validate label entry
        QIntValidator* v = new QIntValidator(this);
        _sg_window->setValidator(v);
        _sg_degree->setValidator(new QIntValidator(this));

    } else if ( _sg_frame->isHidden() ) {
        _sg_frame->show();
        return;
    } else {
        _sg_frame->hide();
        return;
    }
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(_sg_degree);
    layout->addWidget(_sg_window);
    layout->addWidget(_sg_slider);
    _sg_frame->setLayout(layout);

    connect(_sg_slider,SIGNAL(valueChanged(int)),
            this,SLOT(_keyPressGSliderChanged(int)));
    connect(_sg_window,SIGNAL(returnPressed()),
            this,SLOT(_keyPressGLineEditReturnPressed()));
    connect(_sg_degree,SIGNAL(returnPressed()),
            this,SLOT(_keyPressGDegreeReturnPressed()));

    // Get data's dt for sgolay window
    double dtMin = DBL_MAX;
    foreach ( QModelIndex curveIdx, curveIdxs ) {
        CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
        if ( curveModel ) {
            curveModel->map();
            int N = 0;
            double dt = 0.0;
            bool isFirst = true;
            double lastTime = 0.0;
            double lastDt = 0.0;
            ModelIterator* it = curveModel->begin();
            while ( !it->isDone() ) {
                if ( isFirst ) {
                    isFirst = false;
                } else {
                    dt = it->t() - lastTime;
                    if (lastDt > 0.0 && dt > 0.0 && qAbs(dt-lastDt) > 1.0e-9) {
                        QMessageBox msgBox;
                        QString msg = QString(
                                    "Savitzky-Golay filter expects "
                                    "uniform sampling frequency.  "
                                    "Data has variable dt.  Bailing!");
                        msgBox.setText(msg);
                        msgBox.exec();
                        delete it;
                        curveModel->unmap();
                        _sg_frame->hide();
                        return;
                    } else if ( dt == 0.0 ) {
                        QMessageBox msgBox;
                        QString msg = QString(
                                   "Savitzky-Golay filter expects "
                                   "uniform sampling frequency.  "
                                   "Data has two values with same time stamp.  "
                                   "Bailing!");
                        msgBox.setText(msg);
                        msgBox.exec();
                        delete it;
                        curveModel->unmap();
                        _sg_frame->hide();
                        return;
                    }
                    lastDt = dt;
                }
                lastTime = it->t();
                ++N;
                it->next();
            }
            if ( dt > 0 && dt < dtMin ) {
                dtMin = dt;
            }

            double minSamplingRate = 4.0; // Hz
            if ( dtMin > 1/minSamplingRate ) {
                QMessageBox msgBox;
                QString msg = QString(
                            "Attempted filter of data with a low sampling rate "
                            "of %1Hz.  "
                            "S-Golay smoothing function expects "
                            "a sampling frequency greater than %2Hz.  "
                            "Bailing!").arg(1/dtMin).arg(minSamplingRate);
                msgBox.setText(msg);
                msgBox.exec();
                _sg_frame->hide();
                return;
            }

            // Cache off original data for later filtering (use _real as cache)
            curveModel->_real = (double*)malloc(N*sizeof(double));

            it = it->at(0);
            double goodVal = 0.0;
            while ( !it->isDone() ) {
                if ( std::isnan(it->y()) ) {
                    it->next();
                    continue;
                }
                goodVal = it->y();
                break;
            }

            int i = 0;
            it = it->at(0);
            while ( !it->isDone() ) {
                curveModel->_real[i] = it->y();
                if ( std::isnan(curveModel->_real[i]) ) {
                    curveModel->_real[i] = goodVal;
                }
                goodVal = curveModel->_real[i];
                it->next();
                ++i;
            }
        }
    }
    int maxRange = (int)((1.0/dtMin)/5.0);
    if ( maxRange > 999 ) {
        maxRange = 999;
    }
    _sg_slider->setRange(3,maxRange);
    QIntValidator* v = (QIntValidator*)_sg_window->validator();
    v->setRange(3,maxRange);
    v = (QIntValidator*)_sg_degree->validator();
    v->setRange(2,6);
    QString degree = QString("%1").arg(2);
    _sg_degree->setText(degree);
    _sg_slider->setValue((int)(maxRange/3));  // Initial guess that works a lot

    _sg_slider->show();
    _sg_window->show();
    _sg_frame->show();

}

void CurvesView::_keyPressD()
{
    QModelIndex plotIdx = rootIndex();
    QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
    QModelIndexList curveIdxs = _bookModel()->getIndexList(curvesIdx,
                                                           "Curve","Curves");

    // Plot presentation should be compare
    QString plotPresentation = _bookModel()->getDataString(plotIdx,
                                                     "PlotPresentation","Plot");
    if ( plotPresentation != "compare" ) {
        QMessageBox msgBox;
        QString msg = QString("Attempting to take derivative with plot "
                              "presentation=%1.  The derivative only works in "
                              "compare mode.\n").arg(plotPresentation);
        msgBox.setText(msg);
        msgBox.exec();
        return;
    }

    // xunit should be in time
    QModelIndex timeNamesIdx = _bookModel()->getDataIndex(QModelIndex(),
                                                     "TimeNames","");
    QStringList timeNames = _bookModel()->data(timeNamesIdx).toStringList();
    foreach ( QModelIndex curveIdx, curveIdxs ) {
        CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
        if ( !timeNames.contains(curveModel->x()->name()) ) {
            QMessageBox msgBox;
            QString msg = QString("Sorry, attempting take derivative with "
                                  "x=%1.  The integral expects x "
                                  "to be a specified time name. "\
                                  "Try using the -timeName option.\n")
                                  .arg(curveModel->x()->name());
            msgBox.setText(msg);
            msgBox.exec();
            return;
        }
        if ( !Unit::canConvert(curveModel->x()->unit(),"s") ) {
            QMessageBox msgBox;
            QString msg = QString("Sorry, attempting to take derivative with "
                                  "xunit=%1.  The derivative expects the logged"
                                  " xunits to be time.\n")
                                  .arg(curveModel->x()->unit());
            msgBox.setText(msg);
            msgBox.exec();
            return;
        }
    }

    // If user added curves to plot - cache is no longer valid
    if ( !_integCache.plotCaches.isEmpty()) {
        int nCachedCurves = _integCache.plotCaches.last()->curveCaches.size();
        int ncurves = curveIdxs.size();
        if ( nCachedCurves != ncurves ) {
            QMessageBox msgBox;
            QString msg = QString("Curves added after last integration.  "
                                  "Please clear plot and try again.");
            msgBox.setText(msg);
            msgBox.exec();
            return;
        }
    }

    if ( _integCache.plotCaches.isEmpty()) {

        // Cache
        DerivPlotCache* plotCache = new DerivPlotCache();
        plotCache->yAxisLabel = _bookModel()->getDataString(plotIdx,
                                                       "PlotYAxisLabel","Plot");
        plotCache->M = _bookModel()->getPlotMathRect(plotIdx);
        double start = plotCache->M.left();
        double stop = plotCache->M.right();
        foreach ( QModelIndex curveIdx, curveIdxs ) {
            CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
            double xs = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveXScale","Curve");
            double xb = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveXBias","Curve");
            double ys = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveYScale","Curve");
            double yb = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveYBias","Curve");
            QString yUnit = _bookModel()->getDataString(curveIdx,
                                                        "CurveYUnit","Curve");
            QString yLabel = _bookModel()->getDataString(curveIdx,
                                                         "CurveYLabel","Curve");
            CurveCache* curveCache = new CurveCache(curveModel,
                                                    xs,xb,ys,yb,
                                                    yLabel,yUnit);
            plotCache->curveCaches.append(curveCache);
        }
        _derivCache.plotCaches.append(plotCache);

        QProgressDialog progress("Derivative", "Abort", 0,
                                 curveIdxs.size(), this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setMinimumDuration(500);
        int i = 0;

        bool block = _bookModel()->blockSignals(true);
        QString plotUnit = _bookModel()->getCurvesYUnit(curvesIdx);
        QString plotDerivUnit = Unit::derivative(plotUnit);
        foreach ( QModelIndex curveIdx, curveIdxs ) {
            QString xu = _bookModel()->getDataString(curveIdx,
                                                     "CurveXUnit","Curve");
            double xs = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveXScale","Curve");
            double xb = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveXBias","Curve");
            QString yu = _bookModel()->getDataString(curveIdx,
                                                     "CurveYUnit","Curve");
            double ys = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveYScale","Curve");
            double yb = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveYBias","Curve");
            CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
            CurveModel* deriv = new CurveModelDerivative(curveModel,
                                                         timeNames, start,stop,
                                                         xu,xs,xb,yu,ys,yb);
            QVariant v = PtrToQVariant<CurveModel>::convert(deriv);
            QModelIndex curveDataIdx = _bookModel()->getDataIndex(curveIdx,
                                                           "CurveData","Curve");
            QModelIndex yUnitIdx = _bookModel()->getDataIndex(curveIdx,
                                                          "CurveYUnit","Curve");
            _bookModel()->setData(yUnitIdx,"--");  // temp for conversion below
            _bookModel()->setData(curveDataIdx,v);
            if ( Unit::canConvert(deriv->y()->unit(),plotDerivUnit) ) {
                _bookModel()->setData(yUnitIdx,plotDerivUnit);
            } else {
                _bookModel()->setData(yUnitIdx,deriv->y()->unit());
            }

            // Set y scale/bias to 1/0 since scale and bias baked in curve
            QModelIndex yScaleIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYScale","Curve");
            _bookModel()->setData(yScaleIdx,1.0);
            QModelIndex yBiasIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYBias","Curve");
            _bookModel()->setData(yBiasIdx,0.0);

            QModelIndex yNameIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYName","Curve");
            QModelIndex yLabelIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYLabel","Curve");

            _bookModel()->setData(yNameIdx, deriv->y()->name());
            _bookModel()->setData(yLabelIdx, deriv->y()->name());

            progress.setValue(i++);
            if (progress.wasCanceled()) {
                break;
            }
            QString msg = QString("Loaded %1 of %2 curves")
                    .arg(i).arg(curveIdxs.size());
            progress.setLabelText(msg);
        }
        _bookModel()->blockSignals(block);

        // Yaxislabel and refresh plot with new bounding box
        QModelIndex yAxisLabelIdx = _bookModel()->getDataIndex(plotIdx,
                                                       "PlotYAxisLabel","Plot");
        QString yAxisLabel = _bookModel()->data(yAxisLabelIdx).toString();
        QString dYAxisLabel = "d\'(" + yAxisLabel + ")";
        _bookModel()->setData(yAxisLabelIdx,dYAxisLabel);
        QRectF bbox = _bookModel()->calcCurvesBBox(curvesIdx);
        _bookModel()->setPlotMathRect(bbox,rootIndex());

        progress.setValue(curveIdxs.size());
    } else {
        IntegPlotCache* plotCache = _integCache.plotCaches.takeLast();
        if ( _integ_ival ) {
            QString s = QString("%1").arg(plotCache->initialValue);
            _integ_ival->setText(s);
        }
        bool block = _bookModel()->blockSignals(true);
        int i = 0;
        foreach ( QModelIndex curveIdx, curveIdxs ) {
            CurveCache* curveCache = plotCache->curveCaches.at(i++);
            QModelIndex yUnitIdx = _bookModel()->getDataIndex(curveIdx,
                                                          "CurveYUnit","Curve");
            _bookModel()->setData(yUnitIdx,""); // Unset units
            QModelIndex curveDataIdx = _bookModel()->getDataIndex(curveIdx,
                                                           "CurveData","Curve");
            CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
            if ( curveModel ) {
                delete curveModel;
            }
            QVariant v=PtrToQVariant<CurveModel>::convert(
                                                      curveCache->curveModel());
            _bookModel()->setData(curveDataIdx,v);
            _bookModel()->setData(yUnitIdx,curveCache->yUnit()); // Reset units

            QModelIndex xScaleIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveXScale","Curve");
            QModelIndex xBiasIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveXBias","Curve");
            QModelIndex yScaleIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYScale","Curve");
            QModelIndex yBiasIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYBias","Curve");
            QModelIndex yNameIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYName","Curve");
            QModelIndex yLabelIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYLabel","Curve");
            _bookModel()->setData(xScaleIdx, curveCache->xScale());
            _bookModel()->setData(xBiasIdx,  curveCache->xBias());
            _bookModel()->setData(yScaleIdx, curveCache->yScale());
            _bookModel()->setData(yBiasIdx,  curveCache->yBias());
            _bookModel()->setData(yNameIdx,
                                  curveCache->curveModel()->y()->name());
            _bookModel()->setData(yLabelIdx, curveCache->yLabel());
        }
        _bookModel()->blockSignals(block);
        QModelIndex yAxisLabelIdx = _bookModel()->getDataIndex(plotIdx,
                                                       "PlotYAxisLabel","Plot");
        _bookModel()->setData(yAxisLabelIdx,plotCache->yAxisLabel);
        _bookModel()->setPlotMathRect(plotCache->M,plotIdx);
    }

    // Since signals blocked above for performance, refresh yname/ylabels
    foreach ( QModelIndex curveIdx, curveIdxs ) {
        QModelIndex yNameIdx = _bookModel()->getDataIndex(curveIdx,
                                                          "CurveYName","Curve");
        QModelIndex yLabelIdx = _bookModel()->getDataIndex(curveIdx,
                                                           "CurveYLabel","Curve");
        QString yName = _bookModel()->getDataString(curveIdx,
                                                    "CurveYName","Curve");
        QString yLabel = _bookModel()->getDataString(curveIdx,
                                                     "CurveYLabel","Curve");
        _bookModel()->setData(yNameIdx, "");
        _bookModel()->setData(yLabelIdx, "");
        _bookModel()->setData(yNameIdx, yName);
        _bookModel()->setData(yLabelIdx, yLabel);
    }

    if ( _integ_frame && _integCache.plotCaches.isEmpty() ) {
        // Hide integ init value entry box
        _integ_frame->hide();
    }
}

// Integrate
void CurvesView::_keyPressI()
{
    QModelIndex plotIdx = rootIndex();
    QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
    QModelIndexList curveIdxs = _bookModel()->getIndexList(curvesIdx,
                                                           "Curve","Curves");

    QString plotPresentation = _bookModel()->getDataString(plotIdx,
                                                     "PlotPresentation","Plot");
    if ( plotPresentation != "compare" ) {
        QMessageBox msgBox;
        QString msg = QString("Attempting integrate with plot "
                              "presentation=%1.  The integral only works in "
                              "compare mode.\n").arg(plotPresentation);
        msgBox.setText(msg);
        msgBox.exec();
        return;
    }

    // xunit should be time
    QModelIndex timeNamesIdx = _bookModel()->getDataIndex(QModelIndex(),
                                                     "TimeNames","");
    QStringList timeNames = _bookModel()->data(timeNamesIdx).toStringList();
    foreach ( QModelIndex curveIdx, curveIdxs ) {
        CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
        if ( !timeNames.contains(curveModel->x()->name()) ) {
            QMessageBox msgBox;
            QString msg = QString("Sorry, attempting integrate with "
                                  "x=%1.  The integral expects x "
                                  "to be a specified time name. "\
                                  "Try using the -timeName option.\n")
                                  .arg(curveModel->x()->name());
            msgBox.setText(msg);
            msgBox.exec();
            return;
        }
        if ( !Unit::canConvert(curveModel->x()->unit(),"s") ) {
            QMessageBox msgBox;
            QString msg = QString("Sorry, attempting integrate with "
                                  "xunit=%1.  The integral expects the logged"
                                  " xunits to be time.\n")
                                  .arg(curveModel->x()->unit());
            msgBox.setText(msg);
            msgBox.exec();
            return;
        }
    }

    // If user added curves to plot - cache is no longer valid
    if ( !_derivCache.plotCaches.isEmpty()) {
        int nCachedCurves = _derivCache.plotCaches.last()->curveCaches.size();
        int ncurves = curveIdxs.size();
        if ( nCachedCurves != ncurves ) {
            QMessageBox msgBox;
            QString msg = QString("Curves added after last derivative.  "
                                  "Please clear plot and try again.");
            msgBox.setText(msg);
            msgBox.exec();
            return;
        }
    }

    if ( ! _integ_frame && _derivCache.plotCaches.isEmpty()) {
        _integ_frame = new QFrame(this);
        _integ_ival = new QLineEdit(_integ_frame);
        _integ_ival->setText("0.0");
        connect(_integ_ival,SIGNAL(returnPressed()),
                this,SLOT(_keyPressIInitValueReturnPressed()));

        // Shrink text box since default stretches over plot
        QFontMetrics fm(_integ_ival->font());
        int w = fm.averageCharWidth()*16; // 16 is arbitrary
        _integ_ival->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
        _integ_ival->setMinimumWidth(w);
        _integ_ival->setToolTip("Initial integration value");

        // Validate label entry
        QDoubleValidator* v = new QDoubleValidator(this);
        _integ_ival->setValidator(v);

        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(_integ_ival);
        _integ_frame->setLayout(layout);
        _integ_frame->show();
    }

    if ( _derivCache.plotCaches.isEmpty()) {

        // Show integ init value entry box
        if ( _integ_frame ) {
            _integ_frame->show();
        }

        // Cache
        IntegPlotCache* plotCache = new IntegPlotCache();
        plotCache->initialValue = _integ_ival->text().toDouble();
        plotCache->yAxisLabel = _bookModel()->getDataString(plotIdx,
                                                       "PlotYAxisLabel","Plot");
        plotCache->M = _bookModel()->getPlotMathRect(plotIdx);
        foreach ( QModelIndex curveIdx, curveIdxs ) {
            CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
            double xs = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveXScale","Curve");
            double xb = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveXBias","Curve");
            double ys = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveYScale","Curve");
            double yb = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveYBias","Curve");
            QString yLabel = _bookModel()->getDataString(curveIdx,
                                                        "CurveYLabel","Curve");
            QString yUnit = _bookModel()->getDataString(curveIdx,
                                                        "CurveYUnit","Curve");
            CurveCache* curveCache = new CurveCache(curveModel,
                                                    xs,xb,ys,yb,
                                                    yLabel,yUnit);
            plotCache->curveCaches.append(curveCache);
        }
        _integCache.plotCaches.append(plotCache);

        // Integrate
        double ival = _integ_ival->text().toDouble();
        QString plotUnit = _bookModel()->getCurvesYUnit(curvesIdx);
        QString plotIntegUnit = Unit::integral(plotUnit);
        bool block = _bookModel()->blockSignals(true);
        QRectF M = _bookModel()->getPlotMathRect(plotIdx);
        double start = M.left();
        double stop = M.right();
        foreach ( QModelIndex curveIdx, curveIdxs ) {
            QString xu = _bookModel()->getDataString(curveIdx,
                                                     "CurveXUnit","Curve");
            double xs = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveXScale","Curve");
            double xb = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveXBias","Curve");
            QString yu = _bookModel()->getDataString(curveIdx,
                                                     "CurveYUnit","Curve");
            double ys = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveYScale","Curve");
            double yb = _bookModel()->getDataDouble(curveIdx,
                                                     "CurveYBias","Curve");
            CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
            CurveModel* integ = new CurveModelIntegral(curveModel,
                                                       timeNames,
                                                       start,stop,
                                                       xu,xs,xb,yu,ys,yb,ival);
            QVariant v = PtrToQVariant<CurveModel>::convert(integ);
            QModelIndex curveDataIdx = _bookModel()->getDataIndex(curveIdx,
                                                           "CurveData","Curve");
            QModelIndex yUnitIdx = _bookModel()->getDataIndex(curveIdx,
                                                          "CurveYUnit","Curve");
            _bookModel()->setData(yUnitIdx,"--");  // temp for conversion below
            _bookModel()->setData(curveDataIdx,v);
            if ( Unit::canConvert(integ->y()->unit(),plotIntegUnit) ) {
                _bookModel()->setData(yUnitIdx,plotIntegUnit);
            } else {
                _bookModel()->setData(yUnitIdx,integ->y()->unit());
            }

            QModelIndex yNameIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYName","Curve");
            QModelIndex yLabelIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYLabel","Curve");
            _bookModel()->setData(yNameIdx, integ->y()->name());
            _bookModel()->setData(yLabelIdx, integ->y()->name());

            // Reset y scale and bias since baked into integral
            QModelIndex yScaleIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYScale","Curve");
            QModelIndex yBiasIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYBias","Curve");
            _bookModel()->setData(yScaleIdx, 1.0);
            _bookModel()->setData(yBiasIdx, 0.0);
        }
        _bookModel()->blockSignals(block);
        QRectF bbox = _bookModel()->calcCurvesBBox(curvesIdx);
        _bookModel()->setPlotMathRect(bbox,rootIndex());

        // Set YAxisLabel
        QModelIndex yAxisLabelIdx = _bookModel()->getDataIndex(plotIdx,
                                                       "PlotYAxisLabel","Plot");
        QString yAxisLabel = _bookModel()->data(yAxisLabelIdx).toString();
        QChar integSymbol(8747);
        QString iYAxisLabel;
        if ( _integCache.plotCaches.size() == 1 ) {
            iYAxisLabel = QString("%1(%2)").arg(integSymbol).arg(yAxisLabel);
        } else {
            iYAxisLabel = QString("%1%2").arg(integSymbol).arg(yAxisLabel);
        }
        _bookModel()->setData(yAxisLabelIdx,iYAxisLabel);

    } else {
        DerivPlotCache* plotCache = _derivCache.plotCaches.takeLast();
        bool block = _bookModel()->blockSignals(true);
        int i = 0;
        foreach ( QModelIndex curveIdx, curveIdxs ) {
            CurveCache* curveCache = plotCache->curveCaches.at(i++);
            QModelIndex yUnitIdx = _bookModel()->getDataIndex(curveIdx,
                                                          "CurveYUnit","Curve");
            _bookModel()->setData(yUnitIdx,"");  // Unset units
            QModelIndex curveDataIdx = _bookModel()->getDataIndex(curveIdx,
                                                           "CurveData","Curve");
            CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
            if ( curveModel ) {
                delete curveModel;
            }
            QVariant v=PtrToQVariant<CurveModel>::convert(
                                                      curveCache->curveModel());
            _bookModel()->setData(curveDataIdx,v);

            _bookModel()->setData(yUnitIdx,curveCache->yUnit());// Reset units

            QModelIndex xScaleIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveXScale","Curve");
            QModelIndex xBiasIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveXBias","Curve");
            QModelIndex yScaleIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYScale","Curve");
            QModelIndex yBiasIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYBias","Curve");
            QModelIndex yNameIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYName","Curve");
            QModelIndex yLabelIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYLabel","Curve");
            _bookModel()->setData(xScaleIdx, curveCache->xScale());
            _bookModel()->setData(xBiasIdx, curveCache->xBias());
            _bookModel()->setData(yScaleIdx, curveCache->yScale());
            _bookModel()->setData(yBiasIdx, curveCache->yBias());
            _bookModel()->setData(yNameIdx,
                                  curveCache->curveModel()->y()->name());
            _bookModel()->setData(yLabelIdx, curveCache->yLabel());
        }
        _bookModel()->blockSignals(block);
        QModelIndex yAxisLabelIdx = _bookModel()->getDataIndex(plotIdx,
                                                       "PlotYAxisLabel","Plot");
        _bookModel()->setData(yAxisLabelIdx,plotCache->yAxisLabel);
        _bookModel()->setPlotMathRect(plotCache->M,plotIdx);
    }

    // Since signals blocked above for performance, refresh yname/ylabels
    foreach ( QModelIndex curveIdx, curveIdxs ) {
        QModelIndex yNameIdx = _bookModel()->getDataIndex(curveIdx,
                                                          "CurveYName","Curve");
        QModelIndex yLabelIdx = _bookModel()->getDataIndex(curveIdx,
                                                           "CurveYLabel","Curve");
        QString yName = _bookModel()->getDataString(curveIdx,
                                                    "CurveYName","Curve");
        QString yLabel = _bookModel()->getDataString(curveIdx,
                                                     "CurveYLabel","Curve");
        _bookModel()->setData(yNameIdx, "");
        _bookModel()->setData(yLabelIdx, "");
        _bookModel()->setData(yNameIdx, yName);
        _bookModel()->setData(yLabelIdx, yLabel);
    }
}

void CurvesView::_keyPressS()
{
    SumOperation curveOp;
    _combinePlotCurves(curveOp);
}

void CurvesView::_keyPressM()
{
    MagnitudeOperation curveOp;
    _combinePlotCurves(curveOp);
}

void CurvesView::_keyPressE()
{
    LowerBoundOperation lb_curveOp;
    _combinePlotCurves(lb_curveOp);

    UpperBoundOperation ub_curveOp;
    _combinePlotCurves(ub_curveOp);
}

// Takes curves of curve's view plot and generates a new curve on the plot
// by combining the curves using the given "curveOp".
// For example, if curveOp is a summation,
// a new curve is generated on the plot that is the sum of all the
// curves point by point foreach timestamp.
// Notes:
// If curve has a timestamp with a nan y-val, throw out the timestamp
// Handle case when there are duplicate timestamps
// If times do not match, interpolate
void CurvesView::_combinePlotCurves(CurveOperation &curveOp)
{
    QModelIndex plotIdx = rootIndex();
    QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
    QModelIndexList curveIdxs = _bookModel()->getIndexList(curvesIdx,
                                                           "Curve","Curves");
    int nCurvesToSum = curveIdxs.size();

    // Get list of curves to sum
    QList<CurveInfo*> curveInfos;
    foreach ( QModelIndex curveIdx, curveIdxs ) {
        CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
        double xs = _bookModel()->getDataDouble(curveIdx,"CurveXScale");
        double xb = _bookModel()->getDataDouble(curveIdx,"CurveXBias");
        double ys = _bookModel()->getDataDouble(curveIdx,"CurveYScale");
        double yb = _bookModel()->getDataDouble(curveIdx,"CurveYBias");
        QString bmxu = _bookModel()->getDataString(curveIdx,"CurveXUnit");
        QString bmyu = _bookModel()->getDataString(curveIdx,"CurveYUnit");
        if ( curveModel ) {
            curveInfos.append(new CurveInfo{curveModel,xs,xb,ys,yb,bmxu,bmyu});
        }
    }

    if (curveInfos.isEmpty()) {
        return;
    }

    // Map all models
    foreach (CurveInfo* curveInfo, curveInfos) {
        curveInfo->curveModel->map();
    }

    bool isXTime = true;
    foreach (CurveInfo* curveInfo, curveInfos) {
        if ( curveInfo->curveModel->t()->name() !=
             curveInfo->curveModel->x()->name() ) {
            isXTime = false;
            break;
        }
    }

    // Choose x time unit
    QString xUnit;
    if ( isXTime ) {
        foreach (CurveInfo* curveInfo, curveInfos) {
            if ( !curveInfo->bmXUnit.isEmpty() ) {
                xUnit = curveInfo->bmXUnit ;
                break;  // Choose first book model x unit (if it exists)
            } else {
                xUnit = curveInfo->curveModel->x()->unit();
            }
        }
    }

    // Check if y units are all in same family for conversion
    QString yUnit;
    foreach (CurveInfo* curveInfo, curveInfos) {
        if ( yUnit.isEmpty() ) {
            if ( !curveInfo->bmYUnit.isEmpty() ) {
                yUnit = curveInfo->bmYUnit ;
            } else {
                yUnit = curveInfo->curveModel->y()->unit();
            }
        } else {
            if ( !Unit::canConvert(yUnit,curveInfo->curveModel->y()->unit()) ) {
                yUnit.clear();
                break;
            }
        }
    }

    // Hash to keep previous points and unit for each iterator
    QHash<ModelIterator*, QPointF> it2prevPoint;
    QHash<ModelIterator*, CurveInfo*> it2curveInfo;
    // Map each curve and get iterators for each curve
    QList<ModelIterator*> iterators;
    foreach (CurveInfo* curveInfo, curveInfos) {
        ModelIterator* it = curveInfo->curveModel->begin();
        it->start();
        iterators.append(it);
        it2prevPoint.insert(it,QPointF(qQNaN(),qQNaN()));
        it2curveInfo.insert(it,curveInfo);
    }

    // Get time match tolerance for comparing timestamps
    double tmt = _bookModel()->getDataDouble(QModelIndex(),
                                             "TimeMatchTolerance",
                                             "");

    // Calculate beg/end times for all curves
    foreach (ModelIterator* it, iterators) {
        while ( !it->isDone() ) {
            double t = _getTime(isXTime,xUnit,it,it2curveInfo.value(it));
            if ( t < it2curveInfo.value(it)->begTime && !qIsNaN(it->y()) ) {
                it2curveInfo.value(it)->begTime = t;
            }
            if ( t > it2curveInfo.value(it)->endTime && !qIsNaN(it->y()) ) {
                it2curveInfo.value(it)->endTime = t;
            }
            it->next();
        }
    }

    // Reset all iterators to start
    foreach (ModelIterator* it, iterators) {
        it->start();
    }

    // Load the curve combination into a vector of points
    QVector<QPointF>* points = new QVector<QPointF>();
    while (true) {

        bool isDone = true;
        foreach (ModelIterator* it, iterators) {
            if ( !it->isDone() ) {
                isDone = false;
                break;
            }
        }
        if ( isDone ) {
            break;
        }

        double minTime = DBL_MAX;
        foreach (ModelIterator* it, iterators) {
            if ( !it->isDone() ) {
                double t = _getTime(isXTime,xUnit,it,it2curveInfo.value(it));
                if ( t < minTime ) {
                    minTime = t;
                }
            }
        }

        bool isTimeMatch = true;
        foreach (ModelIterator* it, iterators) {
            if (it->isDone()) {
                isTimeMatch = false;
                break;
            }
            double t = _getTime(isXTime,xUnit,it,it2curveInfo.value(it));
            if (qAbs(t-minTime) > tmt) {
                isTimeMatch = false;
                break;
            }
        }

        if (isTimeMatch) {
            // Combine the y-values for matching timestamps
            QVector<double> yvals;
            foreach (ModelIterator* it, iterators) {
                double t = _getTime(isXTime,xUnit,it,it2curveInfo.value(it));
                double unitScale = 1.0;
                double unitBias = 0.0;
                if ( !yUnit.isEmpty() ) {
                    unitScale = it2curveInfo.value(it)->yUnitScale;
                    unitBias  = it2curveInfo.value(it)->yUnitBias;
                }
                double y = it->y()*unitScale + unitBias;
                y *= it2curveInfo.value(it)->yScale;
                y += it2curveInfo.value(it)->yBias;
                yvals.append(y);
                it2prevPoint.insert(it,QPointF(t,y));
            }

            // Accept/load point
            double result = curveOp.compute(yvals);
            if ( !qIsNaN(result) ) {
                points->append(QPointF(minTime,result));
            }

            // Move all iterators to the next point
            foreach (ModelIterator* it, iterators) {
                it->next();
            }
        } else {
            double isOK = true;
            QVector<double> yvals;
            foreach (ModelIterator* it, iterators) {
                double t = 0.0;
                if ( !it->isDone() ) {
                    t = _getTime(isXTime,xUnit,it,it2curveInfo.value(it));
                }
                if (!it->isDone() && qAbs(t-minTime) > tmt ) {
                    // Iterator time doesn't match, try to interpolate.
                    // Interpolate if time inside curve's beg/end times.
                    // Do not extrapolate.
                    if ( minTime >= it2curveInfo.value(it)->begTime &&
                         minTime <= it2curveInfo.value(it)->endTime ) {
                        double unitScale = 1.0;
                        double unitBias = 0.0;
                        if ( !yUnit.isEmpty() ) {
                            unitScale = it2curveInfo.value(it)->yUnitScale;
                            unitBias  = it2curveInfo.value(it)->yUnitBias;
                        }
                        double ys = it2curveInfo.value(it)->yScale;
                        double yb = it2curveInfo.value(it)->yBias;
                        double y = unitScale*it->y()+unitBias;
                        y = ys*y + yb;
                        QPointF p0 = it2prevPoint.value(it);
                        QPointF p1 = QPointF(t,y);
                        double t0 = p0.x();
                        double y0 = p0.y();
                        double t1 = p1.x();
                        double y1 = p1.y();
                        if ( qIsNaN(t0) || qIsNaN(t1) ||
                             qIsNaN(y0) || qIsNaN(y1) ) {
                            isOK = false;
                            break;
                        } else if ( t1-t0 != 0.0 ) {
                            double m = (y1-y0)/(t1-t0);
                            double y = m*(minTime-t0)+y0;
                            yvals.append(y);
                        } else {
                            isOK = false;
                            break;
                        }
                    } else {
                        // Do nothing, but allow combining of points for other
                        // curves that do match instead of breaking out of this
                        // loop for a curve that does not match
                    }
                } else if (!it->isDone() && qAbs(t-minTime) <= tmt ) {
                    // Iterator time matches
                    if ( !qIsNaN(it->y()) ) {
                        double unitScale = 1.0;
                        double unitBias = 0.0;
                        if ( !yUnit.isEmpty() ) {
                            unitScale = it2curveInfo.value(it)->yUnitScale;
                            unitBias  = it2curveInfo.value(it)->yUnitBias;
                        }
                        double y = it->y()*unitScale+unitBias;
                        y *= it2curveInfo.value(it)->yScale;
                        y += it2curveInfo.value(it)->yBias;
                        yvals.append(y);
                    } else {
                        isOK = false;
                        break;
                    }
                } else {
                    // Do nothing, but allow combining of points for other
                    // curves that do match instead of breaking out of this
                    // loop for a curve that does not match
                }
            }
            if ( isOK ) {
                double result = curveOp.compute(yvals);
                if ( !qIsNaN(result) ) {
                    points->append(QPointF(minTime,result));
                }
            }

            // Step iterators that are sitting on minTime
            foreach (ModelIterator* it, iterators) {
                double t = 0.0;
                if ( !it->isDone() ) {
                    t = _getTime(isXTime,xUnit,it,it2curveInfo.value(it));
                }
                if (!it->isDone() && qAbs(t-minTime) <= tmt ) {
                    double unitScale = 1.0;
                    double unitBias = 0.0;
                    if ( !yUnit.isEmpty() ) {
                        unitScale = it2curveInfo.value(it)->yUnitScale;
                        unitBias  = it2curveInfo.value(it)->yUnitBias;
                    }
                    double y = it->y()*unitScale+unitBias;
                    y *= it2curveInfo.value(it)->yScale;
                    y += it2curveInfo.value(it)->yBias;
                    it2prevPoint.insert(it,QPointF(t,y));
                    it->next();
                }
            }
        }
    }

    // Clean up iterators
    qDeleteAll(iterators);

    // Unmap curve models and clean curveInfos
    foreach (CurveInfo* curveInfo, curveInfos) {
        curveInfo->curveModel->unmap();
        delete curveInfo;
    }

    // Create curve model from points
    // PointsModel takes ownership of points
    if ( yUnit.isEmpty() ) {
        yUnit = "--";
    }
    QString xUnitSum("s");
    if ( isXTime ) {
        xUnitSum = xUnit;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    std::unique_ptr<QVector<QPointF>> pointsPtr(points);
    DataModel* dataModel = new PointsModel(pointsPtr.release(),
                                           QString("sys.exec.out.time"),
                                           xUnitSum,
                                           curveOp.name(),
                                           yUnit);
#else
    QScopedPointer<QVector<QPointF>> pointsPtr(points);
    DataModel* dataModel = new PointsModel(pointsPtr.take(),
                                           QString("sys.exec.out.time"),
                                           xUnitSum,
                                           curveOp.name(),
                                           yUnit);
#endif

    CurveModel* curveModel = new CurveModel(dataModel,0,0,1);

    //
    // Add new curve to plot
    //
    bool block = _bookModel()->blockSignals(true);
    QStandardItem* curvesItem = _bookModel()->itemFromIndex(curvesIdx);
    QStandardItem *curveItem = _bookModel()->addChild(curvesItem,"Curve");

    _bookModel()->addChild(curveItem, "CurveRunID", -1);
    _bookModel()->addChild(curveItem, "CurveRunPath",QString("koviz:memory"));
    _bookModel()->addChild(curveItem, "CurveTimeName",curveModel->t()->name());
    _bookModel()->addChild(curveItem, "CurveTimeUnit",curveModel->t()->unit());

    _bookModel()->addChild(curveItem, "CurveXName", curveModel->x()->name());
    _bookModel()->addChild(curveItem, "CurveXUnit", curveModel->x()->unit());
    _bookModel()->addChild(curveItem, "CurveXScale", 1.0);
    _bookModel()->addChild(curveItem, "CurveXBias", 0.0);

    _bookModel()->addChild(curveItem, "CurveYName", curveModel->y()->name());
    _bookModel()->addChild(curveItem, "CurveYLabel", curveModel->y()->name());
    _bookModel()->addChild(curveItem, "CurveYUnit", curveModel->y()->unit());
    _bookModel()->addChild(curveItem, "CurveYScale", 1.0);
    _bookModel()->addChild(curveItem, "CurveYBias", 0.0);

    _bookModel()->addChild(curveItem, "CurveXMinRange", -DBL_MAX);
    _bookModel()->addChild(curveItem, "CurveXMaxRange",  DBL_MAX);
    _bookModel()->addChild(curveItem, "CurveYMinRange", -DBL_MAX);
    _bookModel()->addChild(curveItem, "CurveYMaxRange",  DBL_MAX);
    _bookModel()->addChild(curveItem, "CurveSymbolSize", "");
    QColor color = (nCurvesToSum == 3) ? QColor(177,77,0) : QColor(35,106,26);
    _bookModel()->addChild(curveItem, "CurveColor",color);
    _bookModel()->addChild(curveItem, "CurveLineStyle", "plain");
    _bookModel()->addChild(curveItem, "CurveSymbolStyle", "none");
    _bookModel()->addChild(curveItem, "CurveSymbolSize", "");
    _bookModel()->addChild(curveItem, "CurveSymbolEnd", "none");

    QVariant v = PtrToQVariant<CurveModel>::convert(curveModel);
    _bookModel()->addChild(curveItem, "CurveData", v);

    // Turn signals back on and reset bounding box
    _bookModel()->blockSignals(block);
    QRectF M = _bookModel()->calcCurvesBBox(curvesIdx);
    QRectF E; // Empty set below to force redraw
    _bookModel()->setPlotMathRect(E,plotIdx);
    _bookModel()->setPlotMathRect(M,plotIdx);
}

double CurvesView::_getTime(bool isXTime,
                          const QString& xUnit,
                          ModelIterator* it,
                          const CurveInfo* curveInfo)
{
    double t = it->t();
    if ( isXTime ) {
        QString xu = curveInfo->curveModel->x()->unit();
        double xus = Unit::scale(xu,xUnit);
        double xub = Unit::bias(xu,xUnit);
        t = it->x();
        t = xus*t + xub;
        double xs = curveInfo->xScale;
        double xb = curveInfo->xBias;
        t = xs*t + xb;
    }
    return t;
}

void CurvesView::_keyPressGChange(int window, int degree)
{
    QModelIndex plotIdx = rootIndex();
    QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
    QModelIndexList curveIdxs = _bookModel()->getIndexList(curvesIdx,
                                                           "Curve","Curves");

    bool block = _bookModel()->blockSignals(true);
    foreach ( QModelIndex curveIdx, curveIdxs ) {
        CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
        if ( curveModel ) {
            CurveModel* sg = new CurveModelSG(curveModel,window,degree);
            QVariant v = PtrToQVariant<CurveModel>::convert(sg);
            QModelIndex curveDataIdx = _bookModel()->getDataIndex(curveIdx,
                                                           "CurveData","Curve");
            _bookModel()->setData(curveDataIdx,v);
            curveModel->_real = 0; // Zero out cache since sg now owns it
            delete curveModel;
        }
    }
    _bookModel()->blockSignals(block);

    // Reset bounding box so that plot refreshes (optimizing redraw)
    QRectF Z; // Empty
    QRectF M = _bookModel()->getPlotMathRect(plotIdx);
    _bookModel()->setPlotMathRect(Z,plotIdx);
    _bookModel()->setPlotMathRect(M,plotIdx);
}

void CurvesView::_keyPressGSliderChanged(int value)
{
    QModelIndex plotIdx = rootIndex();
    QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
    QModelIndexList curveIdxs = _bookModel()->getIndexList(curvesIdx,
                                                           "Curve","Curves");

    bool block = _bookModel()->blockSignals(true);
    foreach ( QModelIndex curveIdx, curveIdxs ) {
        CurveModel* curveModel = _bookModel()->getCurveModel(curveIdx);
        if ( curveModel ) {
            CurveModel* sg = new CurveModelSG(curveModel,value,3);
            QVariant v = PtrToQVariant<CurveModel>::convert(sg);
            QModelIndex curveDataIdx = _bookModel()->getDataIndex(curveIdx,
                                                           "CurveData","Curve");
            _bookModel()->setData(curveDataIdx,v);
            curveModel->_real = 0; // Zero out cache since sg now owns it
            delete curveModel;
        }
    }
    _bookModel()->blockSignals(block);

    // Reset bounding box so that plot refreshes (optimizing redraw)
    QRectF Z; // Empty
    QRectF M = _bookModel()->getPlotMathRect(plotIdx);
    _bookModel()->setPlotMathRect(Z,plotIdx);
    _bookModel()->setPlotMathRect(M,plotIdx);

    // Update lineedit label
    QString s = QString("%1").arg(value);
    _sg_window->setText(s);
}

void CurvesView::_keyPressGLineEditReturnPressed()
{
    QString s = _sg_window->text();
    bool ok;
    int value = s.toInt(&ok);
    if ( ok ) {
        _keyPressGSliderChanged(value);
        _sg_slider->setValue(value);
    }
}

void CurvesView::_keyPressGDegreeReturnPressed()
{
    QString s = _sg_degree->text();
    bool ok;
    int degree = s.toInt(&ok);
    if ( ok ) {
        int window = _sg_window->text().toInt(&ok);
        if ( ok ) {
            _keyPressGChange(window,degree);
        }
    }
}

// Re-integrate using initial value from entry box
void CurvesView::_keyPressIInitValueReturnPressed()
{
    bool ok;
    double ival = _integ_ival->text().toDouble(&ok);
    if ( !ok ) {
        // Since there is a validator this should never happen
        fprintf(stderr, "koviz [bad scoobs]:keyPressIInitValueReturnPressed\n");
        exit(-1);
    }

    QModelIndex plotIdx = rootIndex();
    QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
    QModelIndexList curveIdxs = _bookModel()->getIndexList(curvesIdx,
                                                           "Curve","Curves");

    // Integrate from cached state using updated initial value
    QModelIndex timeNamesIdx = _bookModel()->getDataIndex(QModelIndex(),
                                                          "TimeNames","");
    QStringList timeNames = _bookModel()->data(timeNamesIdx).toStringList();
    QRectF M = _bookModel()->getPlotMathRect(plotIdx);
    double start = M.left();
    double stop = M.right();
    IntegPlotCache* cache = _integCache.plotCaches.last();
    int i = 0;
    foreach ( QModelIndex curveIdx, curveIdxs ) {
        // Since redoing integration with new constant,
        // drop back to cached curve to re-integrate.
        // Xunit will not change via integration so it is not cached
        QString xu = _bookModel()->getDataString(curveIdx,
                                                 "CurveXUnit","Curve");
        double xs = cache->curveCaches.at(i)->xScale();
        double xb = cache->curveCaches.at(i)->xBias();
        QString yu = cache->curveCaches.at(i)->yUnit();
        double ys = cache->curveCaches.at(i)->yScale();
        double yb = cache->curveCaches.at(i)->yBias();
        CurveModel* curveModel = cache->curveCaches.at(i)->curveModel();
        CurveModel* integ = new CurveModelIntegral(curveModel,
                                                   timeNames, start, stop,
                                                   xu,xs,xb,yu,ys,yb,ival);
        QVariant v = PtrToQVariant<CurveModel>::convert(integ);
        QModelIndex curveDataIdx = _bookModel()->getDataIndex(curveIdx,
                                                           "CurveData","Curve");
        _bookModel()->setData(curveDataIdx,v);
        ++i;
    }
    QRectF bbox = _bookModel()->calcCurvesBBox(curvesIdx);
    _bookModel()->setPlotMathRect(bbox,rootIndex());
}

void CurvesView::_keyPressMinus()
{
    QModelIndex curveIdx;
    if ( !currentIndex().isValid() ) {
        // If curve not selected, but only a single curve on plot, use it
        QModelIndex plotIdx = rootIndex();
        QModelIndex curvesIdx = _bookModel()->getIndex(plotIdx,"Curves","Plot");
        int nCurves = _bookModel()->rowCount(curvesIdx);
        if ( nCurves == 1 ) {
            curveIdx = _bookModel()->
                       getIndexList(curvesIdx,"Curve","Curves").at(0);
        }
    } else {
        QModelIndex gpidx = currentIndex().parent().parent();
        QString tag = model()->data(currentIndex()).toString();
        if (currentIndex().isValid() && tag == "Curve" && gpidx == rootIndex()){
            // Curve is selected
            curveIdx = currentIndex();
        }
    }

    if ( curveIdx.isValid() ) {
        QModelIndex yScaleIdx = _bookModel()->getDataIndex(curveIdx,
                                                         "CurveYScale","Curve");
        double yScale = -1.0*_bookModel()->data(yScaleIdx).toDouble();
        _bookModel()->setData(yScaleIdx,yScale);

        // Reset bounding box
        QModelIndex plotIdx = rootIndex();
        QModelIndex curvesIdx = curveIdx.parent();
        QRectF M = _bookModel()->calcCurvesBBox(curvesIdx);
        _bookModel()->setPlotMathRect(M,plotIdx);
    } else {
        QMessageBox msgBox;
        QString msg = QString("Please select curve to flip, "
                              "then press the minus key.");
        msgBox.setText(msg);
        msgBox.exec();
    }
}

FFTCurveCache::FFTCurveCache(double xbias, double xscale,
                             CurveModel* curveModel) :
    _xbias(xbias),
    _xscale(xscale),
    _curveModel(curveModel)
{
}

double FFTCurveCache::xbias() const
{
    return _xbias;
}

double FFTCurveCache::xscale() const
{
    return _xscale;
}

CurveModel* FFTCurveCache::curveModel() const
{
    return _curveModel;
}

FFTCache::FFTCache() :
    isCache(false)
{
}

DerivPlotCache::DerivPlotCache()
{
}

DerivPlotCache::~DerivPlotCache()
{
}

DerivCache::DerivCache()
{
}

DerivCache::~DerivCache()
{
    foreach ( DerivPlotCache* plotCache, plotCaches ) {
        foreach ( CurveCache* curveCache, plotCache->curveCaches ) {
            delete curveCache;
        }
        delete plotCache;
    }
}

IntegPlotCache::IntegPlotCache() :
    initialValue(0.0)
{
}

IntegPlotCache::~IntegPlotCache()
{
}

IntegCache::IntegCache()
{
}

IntegCache::~IntegCache()
{
    foreach ( IntegPlotCache* plotCache, plotCaches ) {
        foreach ( CurveCache* curveCache, plotCache->curveCaches ) {
            delete curveCache;
        }
        delete plotCache;
    }
}

CurveCache::CurveCache(CurveModel *curveModel,
                       double xs, double xb, double ys, double yb,
                       const QString &yLabel, const QString &yUnit) :
    _curveModel(curveModel),
    _xs(xs),_xb(xb),_ys(ys),_yb(yb),
    _yLabel(yLabel),_yUnit(yUnit)
{}

CurveModel* CurveCache::curveModel() const
{
    return _curveModel;
}


double CurveCache::xScale() const
{
    return _xs;
}

double CurveCache::xBias() const
{
    return _xb;
}

double CurveCache::yScale() const
{
    return _ys;
}

double CurveCache::yBias() const
{
    return _yb;
}

QString CurveCache::yLabel() const
{
    return _yLabel;
}

QString CurveCache::yUnit() const
{
    return _yUnit;
}
