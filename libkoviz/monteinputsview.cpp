#include "monteinputsview.h"
#include <QHeaderView>

MonteInputsView::MonteInputsView(Runs *runs, QWidget *parent) :
    QTableView(parent),
    _runs(runs)
{
    setSortingEnabled(true);
    sortByColumn(0,Qt::AscendingOrder);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setTextElideMode(Qt::ElideNone);
    setAcceptDrops(true);

    QHeaderView* headerView = horizontalHeader();
    headerView->setTextElideMode(Qt::ElideLeft);
#if QT_VERSION >= 0x050000
    headerView->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    headerView->setResizeMode(QHeaderView::ResizeToContents);
#endif
    connect(headerView,SIGNAL(sectionClicked(int)),
            this,SLOT(_viewHeaderSectionClicked(int)));

}

void MonteInputsView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);

    _selectModel = new QItemSelectionModel(model);
    setSelectionModel(_selectModel);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setFocusPolicy(Qt::ClickFocus);
    setTabKeyNavigation(false);
    verticalHeader()->hide();
    connect(_selectModel,
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this,
            SLOT(_selectModelCurrentChanged(QModelIndex,QModelIndex)));
}

int MonteInputsView::currentRun()
{
    int runId = -1;
    if ( currentIndex().isValid() ) {
        QModelIndex runIdx = model()->sibling(currentIndex().row(),0,
                                              currentIndex());
        bool ok;
        runId = model()->data(runIdx).toInt(&ok);
        if ( !ok ) {
            runId = -1;
        }
    }
    return runId;
}

void MonteInputsView::_selectModelCurrentChanged(
                     const QModelIndex &curr, const QModelIndex &prev)
{
    Q_UNUSED(prev);

    if ( ! curr.model() ) return ;

#if 0
    QModelIndex runIdx = curr.model()->index(curr.row(),0);
    int runId = curr.model()->data(runIdx).toInt();
    _plotBookView->selectRun(runId);
#endif
}


void MonteInputsView::_viewHeaderSectionClicked(int section)
{
    Q_UNUSED(section);

    QModelIndexList selIdxs = _selectModel->selectedIndexes();
    if ( selIdxs.size() > 0 ) {
        QModelIndex idx = selIdxs.at(section);
        scrollTo(idx, QAbstractItemView::PositionAtCenter);
    }
}

void MonteInputsView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void MonteInputsView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void MonteInputsView::dropEvent(QDropEvent *event)
{
    // Accept drop if monte inputs is a list of paths instead of a
    // monte carlo set of inputs i.e. can't drop a MONTE dir on a MONTE dir
    bool isPaths = true;
    int rc = model()->rowCount();
    for (int row = 0; row < rc; ++row) {
        QString runPath = model()->headerData(row, Qt::Vertical).toString();
        QFileInfo fi(runPath);
        if ( !fi.isFile() && !fi.isDir() ) {
            isPaths = false;
            break;
        }
    }
    if ( !isPaths ) {
        QMessageBox msgBox;
        QString msg = QString("Sorry, adding to Monte Carlo Inputs "
                              "and/or coplotting Monte Carlos is unsupported."
                              "Try Options->ClearRuns, then add new path.");
        msgBox.setText(msg);
        msgBox.exec();
        event->ignore();
        return;
    }

    if (event->mimeData()->hasFormat("text/uri-list")) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            QFileInfo fileInfo(url.toLocalFile());
            if (fileInfo.isFile()) {
                _runs->addRun(fileInfo.absoluteFilePath());
            } else if ( fileInfo.isDir() ) {
                _runs->addRun(fileInfo.absoluteFilePath());
            }
        }
        event->acceptProposedAction();
    } else {
        QTableView::dropEvent(event);
    }
}

void MonteInputsView::keyPressEvent(QKeyEvent *event)
{
    if ( currentIndex().row() < 0 ) {
        event->ignore();
        return;
    }

    if ( event->key() == Qt::Key_Delete ) {
        QString runPath = model()->headerData(currentIndex().row(),
                                              Qt::Vertical).toString();
        _runs->deleteRun(runPath);
    } else {
        QTableView::keyPressEvent(event);
    }
}
