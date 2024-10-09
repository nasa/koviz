#include "runswidget.h"

RunsWidget::RunsWidget(Runs *runs,
                       PlotBookModel *bookModel,
                       MonteInputsView *monteInputsView,
                       QWidget *parent) :
    QWidget(parent),
    _runs(runs),
    _bookModel(bookModel),
    _monteInputsView(monteInputsView)
{
    // Search box
    _gridLayout = new QGridLayout(parent);
    _searchBox = new QLineEdit(parent);
    connect(_searchBox,SIGNAL(returnPressed()),
            this,SLOT(_runsSearchBoxReturnPressed()));
    _gridLayout->addWidget(_searchBox,0,0);

    // Dir/File model
    _fileModel = new QFileSystemModel;

    // Set initial runs home
    _runsHome = QDir::currentPath();
    QString exePath = _runsHome + QDir::separator() + "koviz.exe";
    QString iconPath = _runsHome + QDir::separator() + "koviz.ico";
    if (QFile::exists(exePath) && QFile::exists(iconPath)) {
        // If koviz.exe and koviz.ico are in the current path,
        // assume this is being launched from a Windows install location.
        // In this case, set runs home to user's home directory
        _runsHome = QStandardPaths::
                                 writableLocation(QStandardPaths::HomeLocation);
    }
    _fileModel->setRootPath(_runsHome);

    // Filter
    _filterModel = new RunsWidgetFilterProxyModel(_runs);
    _filterModel->setSourceModel(_fileModel);
    _filterModel->setFilterRole(Qt::DisplayRole);
    _filterModel->setFilterKeyColumn(0);
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        _filterModel->setFilterRegularExpression("");
    #else
        _filterModel->setFilterRegExp("");
    #endif
    // File browser view
    _fileTreeView = new QTreeView(this);
    _fileTreeView->setModel(_filterModel);
    _fileTreeView->setDragEnabled(true);
    _fileTreeView->hideColumn(1); // Hide "Size" column
    _fileTreeView->hideColumn(2); // Hide "Type" column
    _fileTreeView->hideColumn(3); // Hide "Date Modified" column
    _gridLayout->addWidget(_fileTreeView,1,0);

    // Set browser root path
    QModelIndex sourceIndex = _fileModel->index(_runsHome);
    QModelIndex proxyIndex = _filterModel->mapFromSource(sourceIndex);
    _fileTreeView->setRootIndex(proxyIndex);
}

RunsWidget::~RunsWidget()
{
    delete _fileModel;
}

void RunsWidget::setRunsHome(const QString &runsHomeDir)
{
    QFileInfo fi(runsHomeDir);
    if ( !fi.isDir() ) {
        return;
    }
    _runsHome = runsHomeDir;
    QModelIndex sourceIndex = _fileModel->index(_runsHome);
    QModelIndex proxyIndex = _filterModel->mapFromSource(sourceIndex);
    _fileTreeView->setRootIndex(proxyIndex);
}

QString RunsWidget::runsHome()
{
    return _runsHome;
}

void RunsWidget::_runsSearchBoxReturnPressed()
{
    QString rx = _searchBox->text();
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        _filterModel->setFilterRegularExpression(rx);
    #else
        _filterModel->setFilterRegExp(rx);
    #endif

    QModelIndex sourceIndex = _fileModel->index(_runsHome);
    QModelIndex proxyIndex = _filterModel->mapFromSource(sourceIndex);
    _fileTreeView->setRootIndex(proxyIndex);
}

bool RunsWidgetFilterProxyModel::filterAcceptsRow(
                                        int row,const QModelIndex &srcIdx) const
{
    if (!sourceModel()) {
        return false;
    }

    QModelIndex index = sourceModel()->index(row, 0, srcIdx);
    QString path = sourceModel()->data(index,
                                     QFileSystemModel::FilePathRole).toString();
    QFileInfo fi(path);

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        const auto SkipEmptyParts = Qt::SkipEmptyParts ;
    #else
        const auto SkipEmptyParts = QString::SkipEmptyParts;
    #endif
    QStringList pathComponents = path.split(QDir::separator(), SkipEmptyParts);

    if ( pathComponents.size() <= 2 ) {
        // If path is root or nearly toplevel, always accept
        return true;
    }

    // Lambda for Qt compatibility
    auto getFilterRegularExpression = [this]() {
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
            return filterRegularExpression();
        #else
            return QRegularExpression(filterRegExp().pattern());
        #endif
    };

    if ( fi.isFile() ) {
        if ( path.contains(getFilterRegularExpression()) &&
             _runs->isValidRunPath(path) ) {
            return true;
        }
    } else if ( fi.isDir() ) {
        if ( _isDirAccept(path,getFilterRegularExpression(),0) ) {
            return true;
        }
    }

    return false;
}

bool RunsWidgetFilterProxyModel::_isDirAccept(const QString &path,
                                              const QRegularExpression &rx,
                                              int depth) const
{
    // For speed sake, only search a single sub-directory
    if (depth > 1 ) {
        return true;
    }

    if ( path.contains(rx) && _runs->isValidRunPath(path) ) {
        return true;  // Path is valid and matches regexp, so accept
    }

    QDir dir(path);

    // Check files within path directory
    QStringList files = dir.entryList(QDir::Files|QDir::NoDotAndDotDot,
                                      QDir::Name);
    foreach (const QString &file, files) {
        QString ffile = path + QDir::separator() + file;
        if (ffile.contains(rx) && _runs->isValidRunPath(ffile) ) {
            // Path contains regex matched file which is also readable by koviz
            return true;
        }
    }

    // Check path subdirectories
    QStringList dirs = dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot,
                                     QDir::Name);
    foreach (const QString &subDir, dirs) {
        QString subDirPath = path + QDir::separator() + subDir;
        if ( _isDirAccept(subDirPath, rx, depth+1) ) {
            return true;
        }
    }

    return false;
}
