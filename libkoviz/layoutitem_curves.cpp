#include "layoutitem_curves.h"

CurvesLayoutItem::CurvesLayoutItem(PlotBookModel* bookModel,
                                   const QModelIndex& plotIdx,
                                   const QModelIndex &currIdx) :
    PaintableLayoutItem(),
    _bookModel(bookModel),
    _plotIdx(plotIdx),
    _currIdx(currIdx)
{
}

CurvesLayoutItem::~CurvesLayoutItem()
{
}

Qt::Orientations CurvesLayoutItem::expandingDirections() const
{
    return (Qt::Horizontal|Qt::Vertical);
}

QRect CurvesLayoutItem::geometry() const
{
    return _rect;
}

bool CurvesLayoutItem::isEmpty() const
{
    return true;
}

QSize CurvesLayoutItem::maximumSize() const
{
    return QSize(0,0);
}

QSize CurvesLayoutItem::minimumSize() const
{
    return QSize(0,0);
}

void CurvesLayoutItem::setGeometry(const QRect &r)
{
    _rect = r;
}

QSize CurvesLayoutItem::sizeHint() const
{
    QSize size(0,0);
    return size;
}

void CurvesLayoutItem::paint(QPainter *painter,
                             const QRect &R, const QRect &RG,
                             const QRect &C, const QRectF &M)
{
    const QRectF RM = mathRect(RG,C,M);
    QTransform T = coordToDotTransform(R,RM);

    painter->save();
    painter->setClipRect(R);

    QModelIndex curvesIdx = _bookModel->getIndex(_plotIdx,"Curves","Plot");
    int nCurves = _bookModel->rowCount(curvesIdx);

    // Print!
    if ( nCurves == 2 ) {
        QString plotPresentation = _bookModel->getDataString(_plotIdx,
                                                     "PlotPresentation","Plot");
        if ( plotPresentation == "compare" ) {
            _printCoplot(T,painter,_plotIdx);
        } else if (plotPresentation == "error" || plotPresentation.isEmpty()) {
            _printErrorplot(T,painter,_plotIdx);
        } else if ( plotPresentation == "error+compare" ) {
            _printErrorplot(T,painter,_plotIdx);
            _printCoplot(T,painter,_plotIdx);
        } else {
            fprintf(stderr,"koviz [bad scoobs]: printCurves() : pres=\"%s\" "
                           "not recognized.\n",
                           plotPresentation.toLatin1().constData());
            exit(-1);
        }
    } else {

        int nElements = 0;
        for ( int i = 0; i < nCurves; ++i ) {
            QModelIndex curveIdx = _bookModel->index(i,0,curvesIdx);
            QPainterPath* path = _bookModel->getPainterPath(curveIdx);
            nElements += path->elementCount();
        }

        if ( nElements > 100000 || nCurves > 64 ) {

            // Use pixmaps to reduce file size
            double rw = R.width()/(double)painter->device()->logicalDpiX();
            double rh = R.height()/(double)painter->device()->logicalDpiY();
            QPixmap nullPixmap(1,1); // used for dpi
            QPainter nullPainter(&nullPixmap);
            int w = qRound(1.8*rw*nullPainter.device()->logicalDpiX());
            int h = qRound(1.8*rh*nullPainter.device()->logicalDpiY());
            QPixmap pixmap(w,h);

            QModelIndex pageIdx = _plotIdx.parent().parent();
            pixmap.fill(_bookModel->pageBackgroundColor(pageIdx));
            QPainter pixmapPainter(&pixmap);
            QPen pen;
            pen.setWidth(0);
            pixmapPainter.setRenderHint(QPainter::Antialiasing);

            QRectF M = _bookModel->getPlotMathRect(_plotIdx);
            double a = w/M.width();
            double b = h/M.height();
            double c = -a*M.x();
            double d = -b*M.y();
            QTransform T( a,    0,
                          0,    b,
                          c,    d);

            for ( int i = 0; i < nCurves; ++i ) {
                QModelIndex curveIdx = _bookModel->index(i,0,curvesIdx);
                QPainterPath* path =_bookModel->getPainterPath(curveIdx);
                if ( path ) {
                    // Line color
                    QColor color(_bookModel->getDataString(curveIdx,
                                                         "CurveColor","Curve"));
                    pen.setColor(color);
                    pixmapPainter.setPen(pen);

                    // Scale transform
                    double xs = _bookModel->getDataDouble(curveIdx,
                                                         "CurveXScale","Curve");
                    double ys = _bookModel->getDataDouble(curveIdx,
                                                         "CurveYScale","Curve");
                    double xb = _bookModel->getDataDouble(curveIdx,
                                                          "CurveXBias","Curve");
                    double yb = _bookModel->getDataDouble(curveIdx,
                                                          "CurveYBias","Curve");
                    QTransform Tscaled(T);
                    Tscaled = Tscaled.scale(xs,ys);
                    Tscaled = Tscaled.translate(xb/xs,yb/ys);
                    pixmapPainter.setTransform(Tscaled);

                    // Line style
                    QString lineStyle = _bookModel->getDataString(curveIdx,
                                                      "CurveLineStyle","Curve");
                    lineStyle = lineStyle.toLower();

                    // Draw curve!
                    if ( lineStyle == "thick_line" ||
                         lineStyle == "x_thick_line" ) {
                        // The transform cannot be used when drawing thick lines
                        QTransform I;
                        pixmapPainter.setTransform(I);
                        double w = pen.widthF();
                        if ( lineStyle == "thick_line" ) {
                            pen.setWidth(5.0);
                        } else if ( lineStyle == "x_thick_line" ) {
                            pen.setWidthF(9.0);
                        } else {
                            fprintf(stderr, "koviz [bad scoobs]: "
                                    "BookView::_paintCurve: bad linestyle\n");
                            exit(-1);
                        }
                        pixmapPainter.setPen(pen);
                        QPointF pLast;
                        for ( int i = 0; i < path->elementCount(); ++i ) {
                            QPainterPath::Element el = path->elementAt(i);
                            QPointF p(el.x,el.y);
                            p = Tscaled.map(p);
                            if  ( i > 0 ) {
                                pixmapPainter.drawLine(pLast,p);
                            }
                            pLast = p;
                        }
                        pen.setWidthF(w);
                        pixmapPainter.setPen(pen);
                        pixmapPainter.setTransform(Tscaled);
                    } else if ( lineStyle == "scatter" ) {
                        QTransform I;
                        pixmapPainter.setTransform(I);
                        double w = pen.widthF();
                        pen.setWidth(3.0);
                        pixmapPainter.setPen(pen);
                        QBrush origBrush = pixmapPainter.brush();
                        QBrush brush(Qt::SolidPattern);
                        brush.setColor(color);
                        pixmapPainter.setBrush(brush);
                        double r = pen.widthF();
                        for ( int i = 0; i < path->elementCount(); ++i ) {
                            QPainterPath::Element el = path->elementAt(i);
                            QPointF p(el.x,el.y);
                            p = Tscaled.map(p);
                            pixmapPainter.drawEllipse(p,r,r);
                        }
                        pen.setWidthF(w);
                        pixmapPainter.setPen(pen);
                        pixmapPainter.setBrush(origBrush);
                        pixmapPainter.setTransform(Tscaled);
                    } else {
                        pixmapPainter.drawPath(*path);
                    }
                }
            }
            QRectF S(pixmap.rect());
            painter->drawPixmap(R,pixmap,S);
        } else {
            _printCoplot(T,painter,_plotIdx);
        }
    }

    // In case of pixmaps, paint overlays last
    _paintGrid(painter,R,RG,C,M);
    paintHLines(painter,R,RG,C,M);
    paintMarkers(painter,R,RG,C,M);

    // Restore the painter state off the painter stack
    painter->restore();

    // Draw legend
    _paintCurvesLegend(R,curvesIdx,painter);
}

void CurvesLayoutItem::_printCoplot(const QTransform& T,
                            QPainter *painter, const QModelIndex &plotIdx)
{
    double start = _bookModel->getDataDouble(QModelIndex(),"StartTime");
    double stop = _bookModel->getDataDouble(QModelIndex(),"StopTime");

    QString plotXScale = _bookModel->getDataString(plotIdx,
                                                   "PlotXScale","Plot");
    QString plotYScale = _bookModel->getDataString(plotIdx,
                                                   "PlotYScale","Plot");
    bool isXLogScale = ( plotXScale == "log" ) ? true : false;
    bool isYLogScale = ( plotYScale == "log" ) ? true : false;

    QList<QPainterPath*> paths;
    QModelIndex curvesIdx = _bookModel->getIndex(plotIdx,"Curves","Plot");
    int rc = _bookModel->rowCount(curvesIdx);
    for ( int i = 0; i < rc; ++i ) {

        QModelIndex curveIdx = _bookModel->index(i,0,curvesIdx);
        QModelIndex curveDataIdx = _bookModel->getDataIndex(curveIdx,
                                                           "CurveData","Curve");
        QVariant v = _bookModel->data(curveDataIdx);
        CurveModel* curveModel =QVariantToPtr<CurveModel>::convert(v);

        if ( curveModel ) {

            double xs = _bookModel->getDataDouble(curveIdx,
                                                  "CurveXScale","Curve");
            double ys = _bookModel->getDataDouble(curveIdx,
                                                  "CurveYScale","Curve");
            double xb = _bookModel->getDataDouble(curveIdx,
                                                  "CurveXBias","Curve");
            double yb = _bookModel->getDataDouble(curveIdx,
                                                  "CurveYBias","Curve");

            double xus = 1.0;
            double xub = 0.0;
            QString bookXUnit = _bookModel->getDataString(
                                                 curveIdx,"CurveXUnit","Curve");
            if ( !bookXUnit.isEmpty() && bookXUnit != "--" ) {
                QString loggedXUnit = curveModel->x()->unit();
                xus = Unit::scale(loggedXUnit, bookXUnit);
                xub = Unit::bias(loggedXUnit, bookXUnit);
            }

            double yus = 1.0;
            double yub = 0.0;
            QString bookYUnit = _bookModel->getDataString(
                                                 curveIdx,"CurveYUnit","Curve");
            if ( !bookYUnit.isEmpty() && bookYUnit != "--" ) {
                QString loggedYUnit = curveModel->y()->unit();
                yus = Unit::scale(loggedYUnit, bookYUnit);
                yub = Unit::bias(loggedYUnit, bookYUnit);
            }

            QPainterPath* path = new QPainterPath;
            paths << path;

            curveModel->map();
            ModelIterator* it = curveModel->begin();

            QPointF lastPoint;
            bool isFirst = true;
            while ( !it->isDone() ) {

                if ( it->t() < start || it->t() > stop ) {
                    it->next();
                    continue;
                }

                double x = it->x();
                x = xus*x + xub;
                x = xs*x+xb;

                double y = it->y();
                y = yus*y + yub;
                y = ys*y + yb;

                if ( isXLogScale ) {
                    if ( x == 0.0 ) {
                        it->next();
                        continue; // log(0) is -inf (skip point)
                    } else {
                        x = log10(qAbs(x));
                    }
                }
                if ( isYLogScale ) {
                    if ( y == 0.0 ) {
                        it->next();
                        continue;  // log(0) is -inf (skip point)
                    } else {
                        y = log10(qAbs(y));
                    }
                }
                QPointF p(x,y);
                lastPoint = p;
                p = T.map(p);

                if ( isFirst ) {
                    path->moveTo(p.x(),p.y());
                    isFirst = false;
                } else {
                    path->lineTo(p);
                }

                it->next();
            }
            delete it;

            // If curve is flat (constant), label with Point=(x,y) or Flatline=y
            QRectF curveBBox = path->boundingRect();
            if ( curveBBox.height() == 0.0 ) {

                QString label;
                if ( path->elementCount() == 0 ) {
                    QRectF R = painter->clipBoundingRect();
                    QPointF p(R.x()+R.width()/2.0,R.y()+R.height()/2.0);
                    painter->drawText(p,"Empty");
                } else {
                    // Get string value of x
                    double x = lastPoint.x();
                    if ( isXLogScale ) {
                        x = pow(10,x);
                    }
                    QString xString = QString("%1").arg(x,0,'g',9);
                    QVariant v(xString);
                    double x2 = v.toDouble();
                    double e = qAbs(x-x2);
                    if ( e > 1.0e-9 ) {
                        // If %.9g loses too much accuracy, use %lf
                        xString = QString("%1").arg(x,0,'f',9);
                    }

                    // Get string value of y
                    double y = lastPoint.y();
                    if ( isYLogScale ) {
                        y = pow(10,y);
                    }
                    QString yString = QString("%1").arg(y,0,'g',9);
                    v = yString;
                    double y2 = v.toDouble();
                    e = qAbs(y-y2);
                    if ( e > 1.0e-9 ) {
                        // If %.9g loses too much accuracy, use %lf
                        yString = QString("%1").arg(y,0,'f',9);
                    }

                    if ( path->elementCount() == 1 ) {
                        // It's a point, not a curve
                        label = QString("Point=(%1,%2)").
                                                      arg(xString).arg(yString);
                    } else {
                        // It's a flatline
                        label = QString("Flatline=%1").arg(yString);
                    }
                    QColor color( _bookModel->getDataString(curveIdx,
                                                        "CurveColor","Curve"));
                    QPen pen = painter->pen();
                    pen.setColor(color);
                    painter->setPen(pen);

                    int h = painter->fontMetrics().descent();
                    QPoint drawPt = curveBBox.topLeft().toPoint()-QPoint(0,h);
                    QRect L = painter->fontMetrics().boundingRect(label);
                    L.moveTo(drawPt);
                    if ( L.right() > painter->viewport().right() ) {
                        // Move label into viewport
                        drawPt = drawPt - QPoint(L.width(),0);
                    }
                    painter->drawText(drawPt,label);
                }
            }

            curveModel->unmap();
        }
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen = painter->pen();

    double xHeight = painter->fontMetrics().xHeight();
    if ( paths.size() > 20 ) {
        pen.setWidthF(xHeight/11.0);// smaller point size if many curves
    } else if ( paths.size() > 5 && paths.size() < 20 ) {
        pen.setWidthF(xHeight/7.0);
    } else if ( paths.size() < 5 ) {
        pen.setWidthF(xHeight/5.0);
    }
    if (pen.widthF() < 1.0 ) {
        pen.setWidthF(0.0); // cosmetic
    }

    int i = 0;
    foreach ( QPainterPath* path, paths ) {
        QModelIndex curveIdx = _bookModel->index(i,0,curvesIdx);
        QColor color( _bookModel->getDataString(curveIdx,
                                                "CurveColor","Curve"));
        pen.setColor(color);
        QString linestyle =  _bookModel->getDataString(curveIdx,
                                                      "CurveLineStyle","Curve");
        pen.setDashPattern(_bookModel->getLineStylePattern(linestyle));

        // Handle thick_line and x_thick_line styles
        QString style = _bookModel->getDataString(curveIdx,
                                                  "CurveLineStyle","Curve");
        style = style.toLower();
        double penWidthOrig = pen.widthF();
        if ( style == "thick_line" ) {
            if ( pen.widthF() == 0.0 ) {
                pen.setWidth(3.0);
            } else {
                pen.setWidth(pen.widthF()*3.0);
            }
        } else if ( style == "x_thick_line" ) {
            if ( pen.widthF() == 0.0 ) {
                pen.setWidth(5.0);
            } else {
                pen.setWidth(pen.widthF()*5.0);
            }
        } else if ( style == "scatter" ) {
            if ( pen.widthF() == 0.0 ) {
                pen.setWidthF(1.0);
            } else {
                pen.setWidthF(pen.widthF()*1.0);
            }
        }
        painter->setPen(pen);

        if ( style == "scatter" ) {
            QBrush origBrush = painter->brush();
            QBrush brush(Qt::SolidPattern);
            brush.setColor(color);
            painter->setBrush(brush);
            for ( int i = 0; i < path->elementCount(); ++i ) {
                QPainterPath::Element el = path->elementAt(i);
                QPointF p(el.x,el.y);
                double r = pen.widthF();
                painter->drawEllipse(p,r,r); // Qt would not drawPoint for me!
            }
            painter->setBrush(origBrush);
        } else {
            painter->drawPath(*path);
        }
        pen.setWidthF(penWidthOrig);

        // Draw symbols
        QString symbolStyle = _bookModel->getDataString(curveIdx,
                                                   "CurveSymbolStyle", "Curve");
        symbolStyle = symbolStyle.toLower();
        if ( !symbolStyle.isEmpty() && symbolStyle != "none" ) {
            QVector<qreal> pattern;
            pen.setDashPattern(pattern); // plain lines for drawing symbols
            double w = pen.widthF();
            pen.setWidthF(xHeight/11.0);
            painter->setPen(pen);
            QPointF pLast;
            for ( int i = 0; i < path->elementCount(); ++i ) {
                QPainterPath::Element el = path->elementAt(i);
                QPointF p(el.x,el.y);
                if ( i > 0 ) {
                    double r = xHeight*3.0;
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
            painter->setPen(pen);
        }

        // Draw end symbol
        QString symbolEnd = _bookModel->getDataString(curveIdx,
                                                   "CurveSymbolEnd", "Curve");
        symbolEnd = symbolEnd.toLower();
        if ( !symbolEnd.isEmpty() && symbolEnd != "none" ) {
            QVector<qreal> pattern;
            pen.setDashPattern(pattern); // plain lines for drawing symbols
            double w = pen.widthF();
            pen.setWidthF(xHeight/11.0);
            painter->setPen(pen);
            int nEls = path->elementCount();
            if ( nEls > 0 ) {
                QPointF pLast = path->elementAt(nEls-1);
                __paintSymbol(pLast,symbolEnd,painter);
            }
            pen.setWidthF(w);
            painter->setPen(pen);
        }


        delete path;
        ++i;
    }
    painter->restore();
}

void CurvesLayoutItem::__paintSymbol(const QPointF &p,
                                     const QString &symbol, QPainter* painter)
{
    QPen origPen = painter->pen();
    QPen pen = painter->pen();
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);

    if ( symbol == "circle" ) {
        painter->drawEllipse(p,36,36);
    } else if ( symbol == "thick_circle" ) {
        pen.setWidth(18.0);
        painter->setPen(pen);
        painter->drawEllipse(p,32,32);
    } else if ( symbol == "solid_circle" ) {
        pen.setWidthF(18.0);
        painter->setPen(pen);
        painter->drawEllipse(p,24,24);
        painter->drawEllipse(p,12,12);
    } else if ( symbol == "square" ) {
        double x = p.x()-30.0;
        double y = p.y()-30.0;
        painter->drawRect(QRectF(x,y,60,60));
    } else if ( symbol == "thick_square") {
        pen.setWidthF(16.0);
        painter->setPen(pen);
        double x = p.x()-30.0;
        double y = p.y()-30.0;
        painter->drawRect(QRectF(x,y,60,60));
    } else if ( symbol == "solid_square" ) {
        pen.setWidthF(16.0);
        painter->setPen(pen);
        double x = p.x()-30.0;
        double y = p.y()-30.0;
        painter->drawRect(QRectF(x,y,60,60));
        pen.setWidthF(24.0);
        painter->setPen(pen);
        x = p.x()-12.0;
        y = p.y()-12.0;
        painter->drawRect(QRectF(x,y,24,24));
    } else if ( symbol == "star" ) { // *
        pen.setWidthF(12.0);
        painter->setPen(pen);
        double r = 36.0;
        QPointF a(p.x()+r*cos(18.0*M_PI/180.0),
                  p.y()-r*sin(18.0*M_PI/180.0));
        QPointF b(p.x(),p.y()-r);
        QPointF c(p.x()-r*cos(18.0*M_PI/180.0),
                  p.y()-r*sin(18.0*M_PI/180.0));
        QPointF d(p.x()-r*cos(54.0*M_PI/180.0),
                  p.y()+r*sin(54.0*M_PI/180.0));
        QPointF e(p.x()+r*cos(54.0*M_PI/180.0),
                  p.y()+r*sin(54.0*M_PI/180.0));
        painter->drawLine(p,a);
        painter->drawLine(p,b);
        painter->drawLine(p,c);
        painter->drawLine(p,d);
        painter->drawLine(p,e);
    } else if ( symbol == "xx" ) {
        pen.setWidthF(12.0);
        painter->setPen(pen);
        QPointF a(p.x()+24.0,p.y()+24.0);
        QPointF b(p.x()-24.0,p.y()+24.0);
        QPointF c(p.x()-24.0,p.y()-24.0);
        QPointF d(p.x()+24.0,p.y()-24.0);
        painter->drawLine(p,a);
        painter->drawLine(p,b);
        painter->drawLine(p,c);
        painter->drawLine(p,d);
    } else if ( symbol == "triangle" ) {
        double r = 48.0;
        pen.setJoinStyle(Qt::MiterJoin);
        painter->setPen(pen);
        QPointF a(p.x(),p.y()-r);
        QPointF b(p.x()-r*cos(30.0*M_PI/180.0),
                  p.y()+r*sin(30.0*M_PI/180.0));
        QPointF c(p.x()+r*cos(30.0*M_PI/180.0),
                  p.y()+r*sin(30.0*M_PI/180.0));
        QPolygonF triangle;
        triangle << a << b << c;
        painter->drawConvexPolygon(triangle);
    } else if ( symbol == "thick_triangle" ) {
        pen.setWidthF(24.0);
        pen.setJoinStyle(Qt::MiterJoin);
        painter->setPen(pen);
        double r = 48.0;
        QPointF a(p.x(),p.y()-r);
        QPointF b(p.x()-r*cos(30.0*M_PI/180.0),
                  p.y()+r*sin(30.0*M_PI/180.0));
        QPointF c(p.x()+r*cos(30.0*M_PI/180.0),
                  p.y()+r*sin(30.0*M_PI/180.0));
        QPolygonF triangle;
        triangle << a << b << c;
        painter->drawConvexPolygon(triangle);
    } else if ( symbol == "solid_triangle" ) {
        pen.setWidthF(36.0);
        pen.setJoinStyle(Qt::MiterJoin);
        painter->setPen(pen);
        double r = 36.0;
        QPointF a(p.x(),p.y()-r);
        QPointF b(p.x()-r*cos(30.0*M_PI/180.0),
                  p.y()+r*sin(30.0*M_PI/180.0));
        QPointF c(p.x()+r*cos(30.0*M_PI/180.0),
                  p.y()+r*sin(30.0*M_PI/180.0));
        QPolygonF triangle;
        triangle << a << b << c;
        painter->drawConvexPolygon(triangle);
    } else if ( symbol.startsWith("number_") && symbol.size() == 8 ) {
        QFont origFont = painter->font();
        QBrush origBrush = painter->brush();

        // Calculate bbox to draw text in
        QString number = symbol.right(1); // last char is '0'-'9'
        QFont font = painter->font();
        font.setPointSize(6);
        painter->setFont(font);
        QFontMetrics fm = painter->fontMetrics();
        QRectF bbox(fm.tightBoundingRect(number));
        bbox.moveCenter(p);

        // Draw solid circle around number
        QRectF box(bbox);
        double l = 3.0*qMax(box.width(),box.height())/2.0;
        box.setWidth(l);
        box.setHeight(l);
        box.moveCenter(p);
        QBrush brush(pen.color());
        painter->setBrush(brush);
        painter->drawEllipse(box);

        // Draw number in white in middle of circle
        QPen whitePen("white");
        painter->setPen(whitePen);
        painter->drawText(bbox,Qt::AlignCenter,number);

        painter->setFont(origFont);
        painter->setBrush(origBrush);
    } else if ( symbol == "satellite" ) {
        // TODO: Really wish that pdf and x11 painting were equivalent
        //       so wouldn't have to have two different drawings
        //       See:  BookIdxView::__paintSymbol()
        QFont origFont = painter->font();
        QBrush origBrush = painter->brush();
        QFont font = painter->font();
        font.setPointSize(160);
        painter->setFont(font);

        painter->save();
        painter->setPen(pen);
        painter->translate(p);
        painter->rotate(37.0);
        QBrush brush(painter->pen().color());
        painter->setBrush(brush);

        // Body
        double bodyWidth = 60.0;
        double bodyHeight = 90.0;
        QRectF bodyRect(-bodyWidth/2.0,-bodyHeight/2.0,bodyWidth,bodyHeight);
        painter->drawRect(bodyRect);

        // Solar Panels
        double panelBarLength = 30.0;
        double panelWidth = 110.0;
        double panelHeight = 50.0;
        QRectF panelRect1(-panelBarLength-bodyWidth/2.0-panelWidth,
                          -panelHeight/2.0,
                          panelWidth,panelHeight);
        QRectF panelRect2(panelBarLength+bodyWidth/2.0,
                          -panelHeight/2.0,
                          panelWidth,panelHeight);
        painter->drawRect(panelRect1);
        painter->drawRect(panelRect2);

        // Panel supports
        painter->drawLine(0,0, bodyWidth/2+panelBarLength,0);
        painter->drawLine(0,0,-bodyWidth/2-panelBarLength,0);

        // Dish support bar
        double dishBarLength = 20.0;
        painter->drawLine(0,bodyHeight/2,
                          0,bodyHeight/2+dishBarLength);

        // Dish
        QRectF dishRect(-bodyWidth/2.0, bodyHeight/2.0+dishBarLength,
                        bodyWidth, bodyWidth);
        painter->drawPie(dishRect,0,180*16);
        double focusHeight = 20.0;
        painter->drawLine(0,bodyHeight/2+bodyWidth/2+dishBarLength,
                         0,bodyHeight/2+bodyWidth/2+dishBarLength+focusHeight);

        painter->setFont(origFont);
        painter->setBrush(origBrush);
        painter->restore();
    }

    painter->setPen(origPen);
}

void CurvesLayoutItem::_printErrorplot(const QTransform& T,
                                       QPainter *painter,
                                       const QModelIndex &plotIdx)
{
    QModelIndex curvesIdx = _bookModel->getIndex(plotIdx,"Curves","Plot");


    // mpath is "math" error path with actual values
    QPainterPath* mpath = _bookModel->getCurvesErrorPath(curvesIdx);

    // Use mpath to create painter path with pdf value points
    QPainterPath path;
    bool isFirst = true;
    for (int i = 0; i < mpath->elementCount(); ++i) {
        QPointF p(mpath->elementAt(i).x,mpath->elementAt(i).y);
        p = T.map(p);
        if ( isFirst ) {
            isFirst = false;
            path.moveTo(p);
        } else {
            path.lineTo(p);
        }
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // Set pen color
    QPen origPen = painter->pen();
    QPen ePen(painter->pen());
    ePen.setWidthF(16.0);
    QRectF curveBBox = path.boundingRect();
    if (curveBBox.height() == 0.0 && path.elementCount() > 0
            && mpath->elementAt(0).y == 0.0) {
        // Color green if error plot is flatline zero
        ePen.setColor(_bookModel->flatLineColor());
    } else {
        ePen.setColor(_bookModel->errorLineColor());
    }
    painter->setPen(ePen);

    if ( curveBBox.height() == 0.0 && path.elementCount() > 0 ) {
        // If curve is flat (constant), label with "Flatline=#"
        QString yval;
        QString plotYScale = _bookModel->getDataString(plotIdx,
                                                       "PlotYScale","Plot");
        double error = mpath->elementAt(0).y;
        if ( plotYScale == "log" ) {
            error = pow(10,error) ;
        }
        if ( curveBBox.width() == 0 ) {
            QString plotXScale = _bookModel->getDataString(plotIdx,
                                                           "PlotXScale","Plot");
            double x = mpath->elementAt(0).x;
            if ( plotXScale == "log" ) {
                x = pow(10,x) ;
            }
            yval = QString("Flatpoint=(%1,%2)").
                    arg(x,0,'g').arg(error,0,'g');
        } else {
            yval = yval.asprintf("Flatline=%.15g",error);
        }
        int h = painter->fontMetrics().descent();
        painter->drawText(curveBBox.topLeft()-QPointF(0,h),yval);
    }

    if ( path.elementCount() == 0 ) {
        QRectF R = painter->clipBoundingRect();
        QPointF p(R.x()+R.width()/2.0,R.y()+R.height()/2.0);
        painter->drawText(p,"Empty");
    } else {
        painter->drawPath(path);  // normal
    }

    painter->setPen(origPen);
    painter->restore();

    delete mpath;
}

void CurvesLayoutItem::_paintGrid(QPainter* painter,
                                  const QRect &R,const QRect &RG,
                                  const QRect &C, const QRectF &M)
{
    // If Grid is off, do not paint grid
    QModelIndex isGridIdx = _bookModel->getDataIndex(_plotIdx,
                                                     "PlotGrid","Plot");
    bool isGrid = _bookModel->data(isGridIdx).toBool();
    if ( !isGrid ) {
        return;
    }

    QString plotXScale = _bookModel->getDataString(_plotIdx,
                                                   "PlotXScale","Plot");
    QString plotYScale = _bookModel->getDataString(_plotIdx,
                                                   "PlotYScale","Plot");
    bool isXLogScale = ( plotXScale == "log" ) ? true : false;
    bool isYLogScale = ( plotYScale == "log" ) ? true : false;

    QList<double> xtics = _bookModel->majorXTics(_plotIdx);
    if ( isXLogScale ) {
        xtics.append(_bookModel->minorXTics(_plotIdx));
    }
    QList<double> ytics = _bookModel->majorYTics(_plotIdx);
    if ( isYLogScale ) {
        ytics.append(_bookModel->minorYTics(_plotIdx));
    }

    QVector<QPointF> vLines;
    QVector<QPointF> hLines;

    const QRectF RM = mathRect(RG,C,M);
    QTransform T = coordToDotTransform(R,RM);

    foreach ( double x, xtics ) {
        vLines << T.map(QPointF(x,RM.top())) << T.map(QPointF(x,RM.bottom()));
    }
    foreach ( double y, ytics ) {
        hLines << T.map(QPointF(RM.left(),y)) << T.map(QPointF(RM.right(),y));
    }

    bool isAntiAliasing = (QPainter::Antialiasing & painter->renderHints()) ;

    // Grid Color
    QModelIndex pageIdx = _bookModel->getIndex(_plotIdx,"Page","Plot");
    QColor color = _bookModel->pageForegroundColor(pageIdx);
    color.setAlpha(40);

    // Pen
    QVector<qreal> dashes;
    qreal space = 4;
    if ( isXLogScale || isYLogScale ) {
        dashes << 1 << 1 ;
    } else {
        dashes << 4 << space ;
    }

    //
    // Draw!
    //
    painter->save();
    painter->setClipRect(R);
    painter->setRenderHint(QPainter::Antialiasing,false);
    QPen origPen = painter->pen();
    QPen pen = painter->pen();
    pen.setColor(color);
    int penWidth = painter->fontMetrics().xHeight()/7;  // 7 arbitrary
    pen.setWidth(penWidth);
    pen.setDashPattern(dashes);
    painter->setPen(pen);
    painter->drawLines(hLines);
    painter->drawLines(vLines);
    painter->setPen(origPen);
    if ( isAntiAliasing ) {
        painter->setRenderHint(QPainter::Antialiasing);
    }
    painter->restore();
}

void CurvesLayoutItem::paintHLines(QPainter *painter,
                                   const QRect &R, const QRect &RG,
                                   const QRect &C, const QRectF &M)
{
    if ( M.width() == 0.0 || M.height() == 0.0 ) {
        return;
    }

    if ( !_bookModel->isChildIndex(_plotIdx,"Plot","HLines") ) {
        return;
    }

    bool isAntiAliasing = (QPainter::Antialiasing & painter->renderHints()) ;

    bool isLogScale = false;
    QString logScale = _bookModel->getDataString(_plotIdx,"PlotYScale","Plot");
    if ( logScale == "log" ) {
        isLogScale = true;
    }

    //
    // Draw!
    //
    QPen origPen = painter->pen();
    QPen pen = painter->pen();
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing,false);
    pen.setWidthF(0.0);
    QModelIndex hlinesIdx = _bookModel->getIndex(_plotIdx,"HLines","Plot");
    QModelIndexList hlineIdxs = _bookModel->getIndexList(hlinesIdx,
                                                           "HLine","HLines");
    const QRectF RM = mathRect(RG,C,M);
    QTransform T = coordToDotTransform(R,RM);
    QTransform I;
    foreach (QModelIndex hlineIdx, hlineIdxs) {

        double val = _bookModel->getDataDouble(hlineIdx,"HLineValue");
        QString valUnit = _bookModel->getDataString(hlineIdx,"HLineUnit");
        QString labelUnit =_bookModel->getDataString(hlineIdx,"HLineLabelUnit");
        QString unit = valUnit;
        if ( Unit::canConvert(valUnit,labelUnit) ) {
            double scale = Unit::scale(valUnit,labelUnit);
            double bias  = Unit::bias(valUnit,labelUnit);
            unit  = labelUnit;
            val = scale*val + bias;
        }

        double origVal(val);
        if ( isLogScale ) {
            if ( val <= 0 ) {
                continue;  // Do not draw lines if log10(val) is <= 0
            }
            val = log10(qAbs(val));
        }

        QString color = _bookModel->getDataString(hlineIdx,"HLineColor");
        if ( color.isEmpty() ) {
            // Default hline color is brown
            pen.setColor("brown");
        } else {
            pen.setColor(color);
        }
        painter->setPen(pen);

        painter->setTransform(T);
        painter->drawLine(QPointF(M.left(),val),QPointF(M.right(),val));

        QPointF pt(M.left(),val);
        QPointF drawPt;
        if ( val > M.center().y() ) {
            // Draw label under hline
            int h = painter->fontMetrics().ascent();
            drawPt = T.map(pt)+QPoint(0,h);
        } else {
            // Draw label above hline
            int h = painter->fontMetrics().descent();
            drawPt = T.map(pt)-QPoint(0,h);
        }
        painter->setTransform(I);

        QString labelfmt = _bookModel->getDataString(hlineIdx,"HLineLabel");
        if ( labelfmt.isEmpty() ) {
            if ( unit.isEmpty() ) {
                labelfmt = "%g";
            } else {
                labelfmt = "%g {%s}";  // default format for hline label
            }
        }
        QRegularExpression rgxFloat
                          (R"(%[-+0# ]?(\d+|\*)?(\.\d+|\.\*)?[lL]?[fFeEgGaA])");
        int idxVal = labelfmt.indexOf(rgxFloat);
        int idxUnit = labelfmt.indexOf("%s");
        if ( idxUnit >= 0 ) {
            labelfmt.replace(idxUnit, 2, unit);
        }
        QString label(labelfmt);
        if ( idxVal >= 0 ) {
            QRegularExpression rgxEndFloat(R"([fFeEgGaA])");
            int idxValEnd = labelfmt.indexOf(rgxEndFloat,idxVal);
            QString valFmt = labelfmt.mid(idxVal,idxValEnd-idxVal+1);
            QString valStr;
            valStr = valStr.asprintf(valFmt.toLatin1().constData(),origVal);
            label.replace(idxVal,idxValEnd-idxVal+1,valStr);
        }

        double placement = _bookModel->getDataDouble(hlineIdx,
                                                     "HLineLabelPlacement");
        QFontMetrics fm = painter->fontMetrics();
        QRect txtRect = fm.boundingRect(label);
        if ( placement == 0 ) {
            // drawPt is on left, and already set, so nothing to do
        } else if ( placement == 1 ) {
            QPointF p(M.right(),0);
            p = T.map(p);
            drawPt.setX(p.x()-txtRect.width());
        } else {
            QPointF p(placement*(M.right()-M.left()),0);
            p = T.map(p);
            drawPt.setX(p.x()-txtRect.width()/2);
            if ( drawPt.x() < 0 ) {
                // Label would go offscreen on left, clamp to 0
                drawPt.setX(0);
            } else if ( (drawPt.x()+txtRect.width()) > R.right() ) {
                // Label would go offscreen on right, clamp to right
                QPointF p(M.right(),0);
                p = T.map(p);
                drawPt.setX(p.x()-txtRect.width());
            }
        }

        painter->drawText(drawPt,label);
    }
    painter->setPen(origPen);
    if ( isAntiAliasing ) {
        painter->setRenderHint(QPainter::Antialiasing);
    }
    painter->restore();
}

void CurvesLayoutItem::paintMarkers(QPainter *painter,
                                    const QRect &R, const QRect &RG,
                                    const QRect &C, const QRectF &M)
{
    // Return and do not draw if Options->isShowLiveCoord is off
    QModelIndex isShowIdx = _bookModel->getDataIndex(QModelIndex(),
                                                     "IsShowLiveCoord");
    bool isShowLiveCoord = _bookModel->data(isShowIdx).toBool();
    if ( !isShowLiveCoord ) {
        return;
    }

    // Draw in window coords
    painter->save();
    QTransform I;
    painter->setTransform(I);

    // Paint markers (if possible)
    QList<TimeAndIndex2*> markers = _getMarkers();
    foreach ( TimeAndIndex2* marker, markers ) {
        QPainterPath* path = _getMarkerPath(marker);
        if ( !path ) continue;

        ScaleBias sb = _getMarkerScaleBias(marker);

        int i = _getMarkerPathIdx(marker,sb,path);
        if ( i < 0 ) continue;

        _paintMarker(painter,marker,R,RG,C,M,sb,path,i);

        QString tag = _bookModel->data(marker->modelIdx()).toString();
        if ( tag == "Plot" ) {
            // Clean error plot path
            delete path;
        }

    }

    // Clean up
    foreach ( TimeAndIndex2* marker, markers ) {
        delete marker;
    }
    painter->restore();
}

// Client must clean allocated markers in returned list
QList<TimeAndIndex2*> CurvesLayoutItem::_getMarkers() const
{
    QList<TimeAndIndex2*> markers;

    QString pres = _bookModel->getDataString(_plotIdx,
                                             "PlotPresentation","Plot");
    QString tag = _bookModel->data(_currIdx).toString();

    // Get live time marker
    QModelIndex liveIdx = _bookModel->getDataIndex(QModelIndex(),
                                                   "LiveCoordTime");
    double liveTime = _bookModel->data(liveIdx).toDouble();
    QModelIndex liveTimeIdxIdx = _bookModel->getDataIndex(QModelIndex(),
                                                          "LiveCoordTimeIndex");
    int timeIdx = _bookModel->data(liveTimeIdxIdx).toInt();
    if ( _currIdx.isValid() && (tag == "Curve" || tag == "Plot") ) {
        TimeAndIndex2* liveMarker = 0;
        if ( tag == "Plot" ) {
            if ( pres == "error" || pres == "error+compare") {
                liveMarker = new TimeAndIndex2(liveTime,timeIdx,_currIdx);
            }
        } else if ( tag == "Curve" ) {
            if ( pres == "compare" || pres == "error+compare" ) {
                liveMarker = new TimeAndIndex2(liveTime,timeIdx,_currIdx);
            } else if ( pres == "error" ) {
                QModelIndex plotIdx = _currIdx.parent().parent();
                liveMarker = new TimeAndIndex2(liveTime,0,plotIdx);
            }
        } else {
            // Should not happen
            fprintf(stderr, "koviz [bad scoobs] "
                            "CurvesLayoutItem::_getMarkers\n");
            exit(-1);
        }
        if ( liveMarker ) {
            markers << liveMarker;
        }
    }

    // Get plot markers
    if ( pres == "error" ) {
        if ( _bookModel->isChildIndex(_plotIdx,"Plot","Markers") ) {
            QModelIndex markersIdx = _bookModel->getIndex(_plotIdx,
                                                          "Markers","Plot");
            QModelIndexList markerIdxs = _bookModel->getIndexList(markersIdx,
                                                            "Marker","Markers");

            foreach ( QModelIndex markerIdx, markerIdxs ) {
                double markerTime = _bookModel->getDataDouble(markerIdx,
                                                         "MarkerTime","Marker");
                int markerTimeIdx = _bookModel->getDataInt(markerIdx,
                                                      "MarkerTimeIdx","Marker");
                TimeAndIndex2* marker = new TimeAndIndex2(markerTime,
                                                        markerTimeIdx,_plotIdx);
                markers << marker;
            }
        }
    }

    // Get curve markers
    if ( pres == "compare" ) {
        QModelIndex curvesIdx = _bookModel->getIndex(_plotIdx,"Curves","Plot");
        QModelIndexList curveIdxs = _bookModel->getIndexList(curvesIdx,
                                                             "Curve","Curves");
        foreach ( QModelIndex curveIdx, curveIdxs ) {
            if ( _bookModel->isChildIndex(curveIdx,"Curve","Markers") ) {
                QModelIndex markersIdx = _bookModel->getIndex(curveIdx,
                                                             "Markers","Curve");
                QModelIndexList markerIdxs =_bookModel->getIndexList(markersIdx,
                                                            "Marker","Markers");
                foreach ( QModelIndex markerIdx, markerIdxs ) {
                    double markerTime = _bookModel->getDataDouble(markerIdx,
                                                         "MarkerTime","Marker");
                    int markerTimeIdx = _bookModel->getDataInt(markerIdx,
                                                      "MarkerTimeIdx","Marker");
                    TimeAndIndex2* marker = new TimeAndIndex2(markerTime,
                                                        markerTimeIdx,curveIdx);
                    markers << marker;
                }
            }
        }
    }

    return markers;
}

ScaleBias CurvesLayoutItem::_getMarkerScaleBias(TimeAndIndex2* marker) const
{
    QString tag = _bookModel->data(marker->modelIdx()).toString();

    // Get plot x/y scale (log or linear)
    QString plotXScale = _bookModel->getDataString(_plotIdx,
                                                   "PlotXScale","Plot");
    QString plotYScale = _bookModel->getDataString(_plotIdx,
                                                   "PlotYScale","Plot");
    bool isXLogScale = (plotXScale=="log") ? true : false;
    bool isYLogScale = (plotYScale=="log") ? true : false;

    // Scale and bias
    double xs = 1.0;
    double ys = 1.0;
    double xb = 0.0;
    double yb = 0.0;
    if ( tag == "Curve") {
        QModelIndex curveIdx = marker->modelIdx();
        if ( !isXLogScale ) {
            // With logscale, scale/bias already done foreach path element
            xs = _bookModel->getDataDouble(curveIdx,"CurveXScale","Curve");
            xb = _bookModel->getDataDouble(curveIdx,"CurveXBias","Curve");
        }
        if ( !isYLogScale ) {
            ys = _bookModel->getDataDouble(curveIdx,"CurveYScale","Curve");
            yb = _bookModel->getDataDouble(curveIdx,"CurveYBias","Curve");
        }
    }

    ScaleBias sb(isXLogScale,isYLogScale,xs,xb,ys,yb);

    return sb;
}

// Get painterpath that marker points to
//
// Unfortunately client needs to clean returned path if path is
// an error plot i.e. it's a path associated with plot index
QPainterPath *CurvesLayoutItem::_getMarkerPath(TimeAndIndex2 *marker) const
{
    QPainterPath* path = 0;

    QString tag = _bookModel->data(marker->modelIdx()).toString();

    if ( tag == "Curve" ) {
        QModelIndex curveIdx = marker->modelIdx();
        path = _bookModel->getPainterPath(curveIdx);
    } else if ( tag == "Plot" ) {
        QModelIndex plotIdx = marker->modelIdx();
        QModelIndex curvesIdx = _bookModel->getIndex(plotIdx,"Curves","Plot");
        path = _bookModel->getCurvesErrorPath(curvesIdx);
    } else {
        fprintf(stderr, "koviz [bad scoobs]: CurvesView::_paintMarkers : "
                        "this case should not happen.\n");
        exit(-1);
    }
    if ( path->elementCount() == 0 ) {
        if ( tag == "Plot" ) {
            delete path; // error path created on the fly!
            path = 0;
        }
    }

    return path;
}

int CurvesLayoutItem::_getMarkerPathIdx(TimeAndIndex2 *marker,
                                        const ScaleBias& sb,
                                        QPainterPath *path) const
{
    QString tag = _bookModel->data(marker->modelIdx()).toString();

    // Get path time (t)
    double t;
    if ( sb.xb != 0.0 || sb.xs != 1.0 ) {
        t = (marker->time()-sb.xb)/sb.xs;
        ROUNDOFF(t,t);
    } else {
        t = marker->time();
        if ( sb.isXLogScale ) {
            t = log10(t);
        }
    }

    // Get element index (i) for time (t)
    int high = path->elementCount()-1;
    int i = _idxAtTimeBinarySearch(path,0,high,t);

    /* There may be duplicate timestamps in sequence - go to first */
    double elementTime = path->elementAt(i).x;
    while ( i > 0 ) {
        if ( path->elementAt(i-1).x == elementTime ) {
            --i;
        } else {
            break;
        }
    }

    // If timestamps identical, it may be necessary to add LiveCoordTimeidx
    int ii = marker->timeIdx();
    i = i+ii;  // LiveCoordTimeIndex is normally 0

    if ( tag == "Curve" ) {
        QModelIndex curveIdx = marker->modelIdx();
        CurveModel* curveModel = _bookModel->getCurveModel(curveIdx);
        curveModel->map();
        int nels = path->elementCount();
        int npts = curveModel->rowCount();
        if ( !_bookModel->isXTime(_plotIdx) || nels != npts ) {
            // If X is not time, then x is e.g. xpos in ball xy orbit
            //
            // If nels != npts, the number of elements in the path do not
            // match the number points in the data - most likely culled
            // due to start/stop time or log eliminating zeroes
            //
            // i is calculated from the curve model instead of the path
            // since the path does not have time
            //
            // i is a best guess

            double xus = 1.0;
            double xub = 0.0;
            QString bookXUnit = _bookModel->getDataString(curveIdx,
                                                          "CurveXUnit","Curve");
            if ( !bookXUnit.isEmpty() && bookXUnit != "--" ) {
                QString loggedXUnit = curveModel->x()->unit();
                xus = Unit::scale(loggedXUnit, bookXUnit);
                xub = Unit::bias(loggedXUnit, bookXUnit);
            }

            double xb = _bookModel->getDataDouble(curveIdx,
                                                    "CurveXBias","Curve");
            double xs = _bookModel->getDataDouble(curveIdx,
                                                    "CurveXScale","Curve");
            if ( _bookModel->isXTime(_plotIdx) ) {
                // Take time shift and scale into account
                // Also take x time unit scale into account
                // and no need for x unit bias since time has no bias
                i = curveModel->indexAtTime((marker->time()-xb)/xs/xus);
            } else {
                i = curveModel->indexAtTime(marker->time());
            }

            if ( nels < npts ) {
                // Points have been culled out of the data leaving the
                // path with less elements than the curve
                // This can happen when:
                //    1) Log(curve) removes zeroes
                //    2) Frequency is given, points culled out of curve
                //    3) Start/stop time given, points lopped off ends
                // Search for first element that matches curve point at time
                ModelIterator* it = curveModel->begin();
                double x = it->at(i)->x();
                double y = it->at(i)->y();
                delete it;

                // Since X unit scale baked in path, need to unit scale
                x = xus*x + xub;    // X Unit scale

                // Since Y unit scale baked in path, need to unit scale
                double yb = _bookModel->getDataDouble(curveIdx,
                                                      "CurveYBias","Curve");
                double ys = _bookModel->getDataDouble(curveIdx,
                                                      "CurveYScale","Curve");
                double yus = 1.0;
                double yub = 0.0;
                QString bookYUnit = _bookModel->getDataString(curveIdx,
                                                              "CurveYUnit","Curve");
                if ( !bookYUnit.isEmpty() && bookYUnit != "--" ) {
                    QString loggedYUnit = curveModel->y()->unit();
                    yus = Unit::scale(loggedYUnit, bookYUnit);
                    yub = Unit::bias(loggedYUnit, bookYUnit);
                }
                y = yus*y + yub;  // Y Unit scale

                if ( sb.isXLogScale ) {
                    // If logscale, scale and bias baked in path
                    // Otherwise, scale and bias in path transform
                    // not in path
                    x = xs*x + xb;
                    x = log10(x);
                }
                if ( sb.isYLogScale ) {
                    y = ys*y + yb;    // Book scale
                    y = log10(y);
                }
                int j = (i < nels) ? i : nels - 1;
                bool isFound = false;
                while ( j >= 0 ) {
                    QPainterPath::Element el = path->elementAt(j);
                    if ( el.x == x && el.y == y ) {
                        i = j;
                        isFound = true;
                        break;
                    }
                    --j;
                }
                if ( !isFound || i >= nels ) {
                    // Point not found, bail
                    curveModel->unmap();
                    return -1;
                }
            } else if ( nels > npts ) {
                // There are more elements in the path than points
                // on the curve.  Shouldn't happen, but if it does
                // don't show marker
                curveModel->unmap();
                return -1;
            }

            /* There may be duplicate timestamps in sequence - goto first */
            ModelIterator* it = curveModel->begin();
            double iTime = it->at(i)->t();
            while ( i > 0 ) {
                if ( it->at(i-1)->t() == iTime ) {
                    --i;
                } else {
                    break;
                }
            }
            delete it;
            i = i + ii;
        }
        curveModel->unmap();
    }

    return i;
}

// T is coordToDot transform
// R is curves viewport rectangle
// Argument i is path index to paint marker at
void CurvesLayoutItem::_paintMarker(QPainter* painter,
                                    TimeAndIndex2 *marker,
                                    const QRect& R,
                                    const QRect& RG,
                                    const QRect& C,
                                    const QRectF& M,
                                    const ScaleBias& sb,
                                    QPainterPath *path, int i)
{
    // Element/coord at marker time
    QPainterPath::Element el = path->elementAt(i);
    QPointF coord(el.x*sb.xs+sb.xb,el.y*sb.ys+sb.yb);

    // Init arrow struct
    CoordArrow arrow(painter);
    arrow.coord = coord;

    // Set arrow text (special syntax for extremums)
    QString x=sb.isXLogScale ? _format(pow(10,coord.x())) : _format(coord.x());
    QString y=sb.isYLogScale ? _format(pow(10,coord.y())) : _format(coord.y());
    int rc = path->elementCount();
    if ( i > 0 && i < rc-1) {
        // First and last point not considered
        double yPrev = path->elementAt(i-1).y*sb.ys+sb.yb;
        double yi = path->elementAt(i).y*sb.ys+sb.yb;
        double yNext = path->elementAt(i+1).y*sb.ys+sb.yb;
        if ( (yi>yPrev && yi>yNext) || (yi<yPrev && yi<yNext) ) {
            arrow.txt = QString("<%1, %2>").arg(x).arg(y);
        } else if ( yPrev == yi && yi != yNext ) {
            arrow.txt = QString("(%1, %2]").arg(x).arg(y);
        } else if ( yPrev != yi && yi == yNext ) {
            arrow.txt = QString("[%1, %2)").arg(x).arg(y);
        } else {
            arrow.txt = QString("(%1, %2)").arg(x).arg(y);
        }
    } else if ( i == 0 ) {
        arrow.txt = QString("init=(%1, %2)").arg(x).arg(y);
    } else if ( i == rc-1 ) {
        arrow.txt = QString("last=(%1, %2)").arg(x).arg(y);
    } else {
        fprintf(stderr,"koviz [bad scoobs]: CurvesView::_paintMarkers()\n");
        exit(-1);
    }

    // Show run name if more than 5 runs
    QString tag = _bookModel->data(marker->modelIdx()).toString();
    if ( tag == "Curve") {
        QModelIndex curveIdx = marker->modelIdx();
        if ( _bookModel->rowCount(curveIdx.parent()) > 5 ) {
            CurveModel* curveModel = _bookModel->getCurveModel(curveIdx);
            if ( curveModel ) {
                QString runPath = curveModel->runPath();
                QFileInfo fi(runPath);
                QString runName;
                foreach (QString name, fi.absoluteFilePath().split('/') ) {
                    if ( name.startsWith("RUN_") ) {
                        runName = name;
                        break;
                    }
                }
                QString runLabel = QString("%1: ").arg(runName);
                arrow.txt.prepend(runLabel);
            }
        }
    }

    // Transform from math coord to pdf/window pt
    const QRectF RM = mathRect(RG,C,M);
    QTransform T = coordToDotTransform(R,RM);

    // Try to fit arrow into viewport using different angles
    QList<double> angles;
    angles << 1*(M_PI/4) << 3*(M_PI/4) << 5*(M_PI/4) << 7*(M_PI/4);
    bool isFits = false;
    foreach ( double angle, angles ) {
        arrow.angle = angle;
        QRect arrowBBox = arrow.boundingBox(*painter,T).toRect();
        if ( R.contains(arrowBBox) ) {
            isFits = true;
            break;
        }
    }

    // Paint!
    QModelIndex pageIdx = _plotIdx.parent().parent();
    QColor bg = _bookModel->pageBackgroundColor(pageIdx);
    QColor fg = _bookModel->pageForegroundColor(pageIdx);
    if ( isFits ) {
        arrow.paintMe(*painter,T,fg,bg);
    } else if ( marker->modelIdx() == _currIdx ) {
        arrow.paintMeCenter(*painter,T,R,fg,bg);
    }
}

QString CurvesLayoutItem::_format(double d)
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

int CurvesLayoutItem::_idxAtTimeBinarySearch(QPainterPath* path,
                                           int low, int high, double time) const
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

void CurvesLayoutItem::_paintCurvesLegend(const QRect& R,
                                          const QModelIndex &curvesIdx,
                                          QPainter* painter)
{
    const int maxEntries = 7;

    QString isShowPlotLegend = _bookModel->getDataString(QModelIndex(),
                                                         "IsShowPlotLegend","");

    int nCurves = _bookModel->rowCount(curvesIdx);
    if ( isShowPlotLegend == "no"  ||
        (isShowPlotLegend == "" && (nCurves > maxEntries || nCurves <= 1)) ) {
        return;
    }

    // If all plots on the page have the same legend, PageTitle will show legend
    if (_bookModel->isPlotLegendsSame(curvesIdx.parent().parent().parent())
        && isShowPlotLegend != "yes" ) {
        return;
    }

    // If all labels on plot are identical, do not show plot legend
    // (unless explicitly set via -showPlotLegend option)
    QModelIndex plotIdx = curvesIdx.parent();
    QStringList labels = _bookModel->legendLabels(plotIdx);
    if ( labels.size() > 1 ) {
        bool isLabelsSame = true;
        foreach ( QString label, labels ) {
            if ( label != labels.at(0) ) {
                isLabelsSame = false;
                break;
            }
        }
        if ( isLabelsSame && isShowPlotLegend != "yes" ) {
            return;
        }
    }

    QString pres = _bookModel->getDataString(curvesIdx.parent(),
                                             "PlotPresentation","Plot");
    if ( pres == "error" ) {
        return;
    }

    QList<QPen*> pens = _bookModel->legendPens(plotIdx,
                                               painter->paintEngine()->type());
    QStringList symbols = _bookModel->legendSymbols(plotIdx);

    if ( pres == "error+compare" ) {
        QPen* magentaPen = new QPen(_bookModel->errorLineColor());
        pens << magentaPen;
        symbols << "none";
        labels << "error";
    }

    __paintCurvesLegend(R,curvesIdx,pens,symbols,labels,painter);

    // Clean up
    foreach ( QPen* pen, pens ) {
        delete pen;
    }
}

// pens,symbols and labels are ordered/collated foreach legend curve/label
void CurvesLayoutItem::__paintCurvesLegend(const QRect& R,
                                           const QModelIndex &curvesIdx,
                                           const QList<QPen *> &pens,
                                           const QStringList &symbols,
                                           const QStringList &labels,
                                           QPainter* painter)
{
    painter->save();
    QPen origPen = painter->pen();

    int n = pens.size();

    // Width Of Legend Box
    const int fw = painter->fontMetrics().averageCharWidth();
    const int ml = fw; // marginLeft
    const int mr = fw; // marginRight
    const int s = fw;  // spaceBetweenLineAndLabel
    const int l = 4*fw;  // line width , e.g. ~5 for: *-----* Gravity
    int w = 0;
    foreach (QString label, labels ) {
        int labelWidth = painter->fontMetrics().boundingRect(label).width();
        int ww = ml + l + s + labelWidth + mr;
        if ( ww > w ) {
            w = ww;
        }
    }

    // Height Of Legend Box
    const int ls = painter->fontMetrics().lineSpacing();
    const int fh = painter->fontMetrics().height();
    const int v = ls/8;  // vertical space between legend entries (0 works)
    const int mt = fh/4; // marginTop
    const int mb = fh/4; // marginBot
    int sh = 0;
    foreach (QString label, labels ) {
        sh += painter->fontMetrics().boundingRect(label).height();
    }
    int h = (n-1)*v + mt + mb + sh;

    // Legend box top left point
    QString pos = _bookModel->getDataString(QModelIndex(),
                                            "PlotLegendPosition","");
    const int tb = fh/4;    // top/bottom margin
    const int rl = fw/2; // right/left margin
    QPoint legendTopLeft;
    if ( pos == "ne" ) { // The default
        legendTopLeft = QPoint(R.right()-w-rl,R.top()+tb);
    } else if ( pos == "e" ) {
        legendTopLeft = QPoint(R.right()-w-rl,R.height()/2-h/2);
    } else if ( pos == "se" ) {
        legendTopLeft = QPoint(R.right()-w-rl,R.height()-h-tb);
    } else if ( pos == "s" ) {
        legendTopLeft = QPoint(R.width()/2-w/2,R.height()-h-tb);
    } else if ( pos == "sw" ) {
        legendTopLeft = QPoint(R.left()+rl,R.height()-h-tb);
    } else if ( pos == "w" ) {
        legendTopLeft = QPoint(R.left()+rl,R.height()/2-h/2);
    } else if ( pos == "nw" ) {
        legendTopLeft = QPoint(R.left()+rl,R.top()+tb);
    } else if ( pos == "n" ) {
        legendTopLeft = QPoint(R.width()/2-w/2,R.top()+tb);
    } else {
        fprintf(stderr, "koviz [bad scoobs]: "
                        "CurvesLayoutItem::__paintCurvesLegend() has "
                        "bad legend position \"%s\"\n",
                pos.toLatin1().constData());
        exit(-1);
    }

    // Legend box
    QRect LegendBox(legendTopLeft,QSize(w,h));

    // Background color
    QModelIndex pageIdx = curvesIdx.parent().parent().parent();
    QColor bg = _bookModel->pageBackgroundColor(pageIdx);
    bg.setAlpha(190);

    // Draw legend box with semi-transparent background
    painter->setBrush(bg);
    QPen penGray(QColor(120,120,120,255));
    painter->setPen(penGray);
    painter->drawRect(LegendBox);
    painter->setPen(origPen);

    QRect lastBB;
    for ( int i = 0; i < n; ++i ) {

        QString label = labels.at(i);

        // Calc bounding rect (bb) for line and label
        QPoint topLeft;
        if ( i == 0 ) {
            topLeft.setX(legendTopLeft.x()+ml);
            topLeft.setY(legendTopLeft.y()+mt);
        } else {
            topLeft.setX(lastBB.bottomLeft().x());
            topLeft.setY(lastBB.bottomLeft().y()+v);
        }
        QRect bb = painter->fontMetrics().boundingRect(label);
        bb.moveTopLeft(topLeft);
        bb.setWidth(l+s+bb.width());

        // Draw line segment
        QPen* pen = pens.at(i);
        painter->setPen(*pen);
        QPoint p1(bb.left(),bb.center().y());
        QPoint p2(bb.left()+l,bb.center().y());
        painter->drawLine(p1,p2);

        // Draw symbols on line segment endpoints
        QString symbol = symbols.at(i);
        __paintSymbol(p1,symbol,painter);
        __paintSymbol(p2,symbol,painter);

        // Draw label
        QRect labelRect(bb);
        QPoint p(bb.topLeft().x()+l+s, bb.topLeft().y());
        labelRect.moveTopLeft(p);
        painter->drawText(labelRect, Qt::AlignLeft|Qt::AlignVCenter, label);

        lastBB = bb;
    }

    painter->setPen(origPen);
    painter->restore();
}
