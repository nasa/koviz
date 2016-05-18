#include "plotview.h"

//  14 parts in a single plot
//  6 rows and 5 columns
//
//    0    1    2            3               4
//  +---++---++---++----------------------++---+
//  |   ||   || C ||        Tics          || C |   0
//  |   ||   |+---++----------------------++---+
//  |   || Y |+---++----------------------++---+
//  | Y ||   ||   ||      Title           ||   |   1
//  |   || T ||   |+----------------------+|   |
//  | A || i ||   |+----------------------+|   |
//  | x || c || T ||                      || T |
//  | i ||   || i ||                      || i |
//  | s || L || c ||        Curves        || c |   2
//  |   || a || s ||                      || s |
//  | L || b ||   ||                      ||   |
//  | a || e ||   ||                      ||   |
//  | b || l ||   ||                      ||   |
//  | e || s |+---++----------------------++---+
//  | l ||   |+---++-----------------------+---+
//  |   ||   || C ||       Tics            | C |   3
//  +---++---++---++-----------------------+---+
//  +------------------------------------------+
//  |                 X Tic Labels             |   4
//  +------------------------------------------+
//  +------------------------------------------+
//  |                 X Axis Label             |   5
//  +------------------------------------------+


PlotView::PlotView(QWidget *parent) :
    BookIdxView(parent),
    _titleView(0),
    _xAxisLabel(0),
    _yAxisLabel(0),
    _tlCorner(0), _trCorner(0), _blCorner(0), _brCorner(0),
    _curvesView(0),
    _tTics(0), _bTics(0), _rTics(0), _lTics(0),
    _xTicLabels(0), _yTicLabels(0)
{
    setFrameShape(QFrame::NoFrame);

    _grid = new QGridLayout;
    _grid->setContentsMargins(0, 0, 0, 0);
    _grid->setSpacing(0);

    _titleView  = new PlotTitleView(this);
    _xAxisLabel = new XAxisLabelView(this);
    _yAxisLabel = new YAxisLabelView(this);
    _tlCorner   = new CornerView(Qt::TopLeftCorner,this);
    _trCorner   = new CornerView(Qt::TopRightCorner,this);
    _blCorner   = new CornerView(Qt::BottomLeftCorner,this);
    _brCorner   = new CornerView(Qt::BottomRightCorner,this);
    _tTics      = new LinedRulerView(Qt::AlignTop,this);
    _bTics      = new LinedRulerView(Qt::AlignBottom,this);
    _rTics      = new LinedRulerView(Qt::AlignRight,this);
    _lTics      = new LinedRulerView(Qt::AlignLeft,this);
    _xTicLabels = new LabeledRulerView(Qt::AlignBottom,this);
    _yTicLabels = new LabeledRulerView(Qt::AlignLeft,this);
    _curvesView = new CurvesView(this);


    _grid->addWidget(_yAxisLabel,0,0,4,1);
    _grid->addWidget(_yTicLabels,0,1,4,1);
    _grid->addWidget(  _tlCorner,0,2,1,1);
    _grid->addWidget(     _tTics,0,3,1,1);
    _grid->addWidget(  _trCorner,0,4,1,1);
    _grid->addWidget(     _lTics,1,2,2,1);
    _grid->addWidget( _titleView,1,3,1,1);
    _grid->addWidget(     _rTics,1,4,2,1);
    _grid->addWidget(_curvesView,2,3,1,1);
    _grid->addWidget(  _blCorner,3,2,1,1);
    _grid->addWidget(     _bTics,3,3,1,1);
    _grid->addWidget(  _brCorner,3,4,1,1);
    _grid->addWidget(_xTicLabels,4,1,1,5);
    _grid->addWidget(_xAxisLabel,5,1,1,5);

    _grid->setRowStretch(0,1);
    _grid->setRowStretch(1,1);
    _grid->setRowStretch(2,100);
    _grid->setRowStretch(3,1);
    _grid->setRowStretch(4,1);
    _grid->setRowStretch(5,1);

    _grid->setColumnStretch(0,1);
    _grid->setColumnStretch(1,1);
    _grid->setColumnStretch(2,1);
    _grid->setColumnStretch(3,100);
    _grid->setColumnStretch(4,1);

    _childViews << _titleView
                << _xAxisLabel
                << _yAxisLabel
                << _curvesView
                << _tlCorner <<  _trCorner << _blCorner << _brCorner
                << _tTics <<  _bTics << _rTics <<  _lTics
                << _xTicLabels << _yTicLabels;

    setLayout(_grid);
}

QSize PlotView::minimumSizeHint() const
{
    QSize s(50,50);
    return s;
}

QSize PlotView::sizeHint() const
{
    QSize s(200,200);
    return s;
}

void PlotView::rowsInserted(const QModelIndex &pidx, int start, int end)
{
    Q_UNUSED(pidx);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

void PlotView::_update()
{
    if ( !_curvesView ) {
        qDebug() << "snap [bad scoobs]: _curvesView should be set in "
                    "PlotView.";
        exit(-1);
    }
    foreach (QAbstractItemView* view, _childViews ) {
        if ( view ) {
            view->setModel(model());
            view->setRootIndex(_myIdx);
        }
        BookIdxView* bview = dynamic_cast<BookIdxView*>(view);
        if ( bview ) {
            bview->setCurvesView(_curvesView);
        }
    }
    if ( model() ) {
        disconnect(model(),SIGNAL(rowsInserted(QModelIndex,int,int)),
                   this,SLOT(rowsInserted(QModelIndex,int,int)));
    }
}
