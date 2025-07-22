#ifndef CURVESVIEW_H
#define CURVESVIEW_H

#include <QtGlobal>
#include <QVector>
#include <QPolygonF>
#include <QPainter>
#include <QPixmap>
#include <QList>
#include <QColor>
#include <QMouseEvent>
#include <QRubberBand>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QSizeF>
#include <QLineF>
#include <QItemSelectionModel>
#include <QEasingCurve>
#include <QImage>
#include <QFontMetrics>
#include <QPoint>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSlider>
#include <QLineEdit>
#include <QIntValidator>
#include <QProgressDialog>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <numeric>
#include "bookidxview.h"
#include "unit.h"
#include "curvemodel.h"
#include "roundoff.h"
#include "layoutitem_curves.h"
#include "coord_arrow.h"
#include "curvemodel_fft.h"
#include "curvemodel_ifft.h"
#include "curvemodel_bw.h"
#include "curvemodel_sg.h"
#include "curvemodel_deriv.h"
#include "curvemodel_integ.h"
#include "datamodel_points.h"

// This is for curve summations
struct CurveInfo
{
    CurveInfo(CurveModel* cm,
              double xs, double xb,
              double ys, double yb,
              const QString& bmxu,
              const QString& bmyu)
            : curveModel(cm),
              xScale(xs), xBias(xb),
              yScale(ys), yBias(yb),
              bmXUnit(bmxu), bmYUnit(bmyu),
              begTime(DBL_MAX),
              endTime(-DBL_MAX)
    {
        QString cmu = cm->y()->unit();
        yUnitScale = 1.0;
        yUnitBias = 0.0;
        if ( !bmyu.isEmpty() ) {
            yUnitScale = Unit::scale(cmu, bmyu);
            yUnitBias  = Unit::bias(cmu, bmyu);
        }
    }

    CurveModel* curveModel;
    double xScale;
    double xBias;
    double yScale;
    double yBias;
    QString bmXUnit; // Book model's x unit (possibly from DP file)
                     // The curveModel->x()->unit() may not be same as bm unit
    QString bmYUnit; // Book model's y unit (possibly from DP file)
                     // The curveModel->y()->unit() may not be same as bm unit

    double yUnitScale; // Calculated in constructor
    double yUnitBias;  // Calculated in constructor

    double begTime;
    double endTime;
};

class CurveOperation {
public:
    virtual QString name() const = 0;
    virtual double compute(const QVector<double>& values) const = 0;
    virtual ~CurveOperation() = default;
};

class SumOperation : public CurveOperation {
public:
    QString name() const override { return "sum"; }
    double compute(const QVector<double>& values) const override {
        return std::accumulate(values.begin(), values.end(), 0.0);
    }
};

class MagnitudeOperation : public CurveOperation {
public:
    QString name() const override { return "magnitude"; }
    double compute(const QVector<double>& values) const override {
        double sumOfSquares = std::accumulate(values.begin(), values.end(), 0.0,
                                      [](double acc, double val)
                                      { return acc + val * val; });
        return std::sqrt(sumOfSquares);
    }
};

class LowerBoundOperation : public CurveOperation {
public:
    QString name() const override { return "lower_bound"; }
    double compute(const QVector<double>& values) const override {
        if (values.isEmpty()) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        return *std::min_element(values.begin(), values.end());
    }
};

class UpperBoundOperation : public CurveOperation {
public:
    QString name() const override { return "upper_bound"; }
    double compute(const QVector<double>& values) const override {
        if (values.isEmpty()) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        return *std::max_element(values.begin(), values.end());
    }
};

class FFTCurveCache
{
  public:
    FFTCurveCache(double xbias, double xscale, CurveModel* curveModel);
    double xbias() const ;
    double xscale() const ;
    CurveModel* curveModel() const ;

  private:
    FFTCurveCache() {}
    double _xbias;
    double _xscale;
    CurveModel*  _curveModel;
};

class FFTCache
{
  public:
    FFTCache();
    bool isCache;
    QString xAxisLabel;
    QRectF M;
    double start;
    double stop;
    QList<FFTCurveCache*> curveCaches;
};

class CurveCache
{
  public:
    CurveCache(CurveModel* curveModel,
               double xs, double xb,
               double ys, double yb,
               const QString& yLabel,
               const QString& yUnit);
    CurveModel* curveModel() const ;
    double xScale() const;
    double xBias() const;
    double yScale() const;
    double yBias() const;
    QString yLabel() const ;
    QString yUnit() const ;

  private:
    CurveCache() {}
    CurveModel*  _curveModel;
    double _xs;
    double _xb;
    double _ys;
    double _yb;
    QString _yLabel;
    QString _yUnit;
};

class DerivPlotCache
{
  public:
    DerivPlotCache();
    ~DerivPlotCache();
    QString yAxisLabel;
    QString yUnit;
    QRectF M;
    QList<CurveCache*> curveCaches;
};

class DerivCache
{
  public:
    DerivCache();
    ~DerivCache();
    QList<DerivPlotCache*> plotCaches;
};

class IntegPlotCache
{
  public:
    IntegPlotCache();
    ~IntegPlotCache();
    double initialValue;
    QString yAxisLabel;
    QString yUnit;
    QRectF M;
    QList<CurveCache*> curveCaches;
};

class IntegCache
{
  public:
    IntegCache();
    ~IntegCache();
    QList<IntegPlotCache*> plotCaches;
};

class CurvesView : public BookIdxView
{
    Q_OBJECT

public:
    explicit CurvesView(QWidget *parent = 0);
    ~CurvesView();

public:
    void setCurrentCurveRunID(int runID) override;

protected:
    void paintEvent(QPaintEvent * event) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void currentChanged(const QModelIndex& current,
                        const QModelIndex& previous) override;
    void resizeEvent(QResizeEvent *event) override;


private:
    QPainterPath* _errorPath;

    QPainterPath _sinPath();
    QPainterPath _stepPath();

    void _paintCoplot(const QTransform& T, QPainter& painter,QPen& pen);
    void _paintErrorplot(const QTransform& T,
                         QPainter& painter, const QPen &pen,
                         const QModelIndex &plotIdx);
    void _paintCurve(const QModelIndex& curveIdx,
                     const QTransform &T, QPainter& painter,
                     bool isHighlight);

    QModelIndex _chooseCurveNearMousePoint(const QPoint& pt);
    bool _isErrorCurveNearMousePoint(const QPoint& pt);

    QPixmap* _pixmap;
    QRectF _lastM;
    bool _isMeasure;
    QPoint _mouseCurrPos;
    QPixmap* _createLivePixmap();

    double _mousePressXBias;
    double _mousePressYBias;

    QString _format(double d);

    int _idxAtTimeBinarySearch(QPainterPath* path,
                               int low, int high, double time);

    // Key Events
    bool _isLastPoint;
    QPointF _lastPoint;
    void _keyPressSpace();
    void _keyPressUp();
    void _keyPressDown();
    void _keyPressPeriod();
    void _keyPressArrow(const Qt::ArrowType& arrow);
    void _keyPressComma();
    void _keyPressEscape();
    void _keyPressF();
    void _keyPressB();
    void _keyPressG();
    void _keyPressD();
    void _keyPressI();
    void _keyPressS();
    void _keyPressM();
    void _keyPressE();
    void _keyPressMinus();

    QFrame* _bw_frame;
    QLineEdit* _bw_label;
    QSlider* _bw_slider;

    QFrame* _sg_frame;
    QLineEdit* _sg_window;
    QLineEdit* _sg_degree;
    QSlider* _sg_slider;
    void _keyPressGChange(int window, int degree);

    QFrame* _integ_frame;
    QLineEdit* _integ_ival;

    FFTCache _fftCache ;
    DerivCache _derivCache ;
    IntegCache _integCache ;

    void _combinePlotCurves(CurveOperation& curveOp);
    double _getTime(bool isXTime, const QString& xUnit,
                    ModelIterator* it, const CurveInfo* curveInfo);

private slots:
    void _keyPressBSliderChanged(int value);
    void _keyPressBLineEditReturnPressed();
    void _keyPressGSliderChanged(int value);
    void _keyPressGLineEditReturnPressed();
    void _keyPressGDegreeReturnPressed();
    void _keyPressIInitValueReturnPressed();

protected slots:
    void dataChanged(const QModelIndex &topLeft,
                     const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;
    void rowsInserted(const QModelIndex &pidx, int start, int end) override;


};

#endif // CURVESVIEW_H
