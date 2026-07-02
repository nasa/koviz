QT  += core
QT  += gui
QT  += xml
QT  += network

# Koviz Version
# Create version.txt and version #DEFINEs for main.cpp
VERSION_TXT = $$PWD/../version.txt
VERSION_GIT_DESC = $$system(git describe --tags --always --dirty)
VERSION_COMMIT_DATE = $$system(git show -s --format=%cI --date=iso-strict)
VERSION_QT_VER = $$[QT_VERSION]
DEFINES += KOVIZ_VERSION=\\\"$$VERSION_GIT_DESC\\\"
DEFINES += KOVIZ_COMMIT_DATE=\\\"$$VERSION_COMMIT_DATE\\\"
DEFINES += KOVIZ_QT_VERSION=\\\"$$VERSION_QT_VER\\\"
system(echo KOVIZ_VERSION = $$VERSION_GIT_DESC > $$VERSION_TXT)
system(echo KOVIZ_COMMIT_DATE = $$VERSION_COMMIT_DATE >> $$VERSION_TXT)
system(echo KOVIZ_QT_VERSION = $$VERSION_QT_VER >> $$VERSION_TXT)

CONFIG -= app_bundle

include($$PWD/../koviz.pri)

release {
    QMAKE_CXXFLAGS_RELEASE -= -g
}

TARGET = koviz
target.path = $$PREFIX/bin
INSTALLS += target

TEMPLATE = app

DESTDIR = $$PWD/../bin
BUILDDIR = $$PWD/../build/$${TARGET}
OBJECTS_DIR = $$BUILDDIR/obj
MOC_DIR     = $$BUILDDIR/moc
RCC_DIR     = $$BUILDDIR/rcc
UI_DIR      = $$BUILDDIR/ui

SOURCES += main.cpp
    
HEADERS +=

OTHER_FILES += 


INCLUDEPATH += $$PWD/..

LIBS += -L$$PWD/../lib -lkoviz

# Ubuntu libs are order dependent so put after -lkoviz
exists( /usr/include/mpv/client.h ) {
    LIBS += -lmpv
}
exists (/usr/include/hdf5/serial/H5Cpp.h) {
    LIBS += -L/usr/lib/x86_64-linux-gnu/hdf5/serial
    LIBS += -lhdf5_cpp -lhdf5
}


PRE_TARGETDEPS += $$PWD/../lib/libkoviz.a
