# =============================================================================
#
# koviz - Trick sim plotting!
#
# =============================================================================

PROJECT_NAME = koviz

# Koviz Version
# Create version #DEFINEs for main.cpp and plotmainwindow.cpp
VERSION_GIT_DESC = $$system(git describe --tags --always --dirty)
VERSION_COMMIT_DATE = $$system(git show -s --format=%cI --date=iso-strict)
VERSION_QT_VER = $$[QT_VERSION]
DEFINES += KOVIZ_VERSION=\\\"$$VERSION_GIT_DESC\\\"
DEFINES += KOVIZ_COMMIT_DATE=\\\"$$VERSION_COMMIT_DATE\\\"
DEFINES += KOVIZ_QT_VERSION=\\\"$$VERSION_QT_VER\\\"
PROJECT_VERSION = $$KOVIZ_VERSION

INSTALL_PREFIX = /usr/local
isEmpty(PREFIX) {
   message("Current install path is '$${INSTALL_PREFIX}'")
   message("To override the install path run: 'qmake PREFIX=/custom/path'")
   PREFIX = $$INSTALL_PREFIX
} else {
   INSTALL_PREFIX = $${PREFIX}
}
INSTALL_LIBDIR = $${PREFIX}/lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
    QT += printsupport
}

exists( /usr/include/mpv/client.h ) {
    DEFINES += HAS_MPV
    LIBS += -lmpv
}

exists (/usr/include/H5Cpp.h) {
    DEFINES += HAS_HDF5
    LIBS += -lhdf5_cpp -lhdf5
}

exists (/usr/include/hdf5/serial/H5Cpp.h) {
    DEFINES += HAS_HDF5
    INCLUDEPATH += /usr/include/hdf5/serial
    # LIBS in koviz.pro
}

# To build: qmake-qt5 ARROW_HOME=/home/kvetter/.local/arrow
isEmpty(ARROW_HOME) {
    ARROW_HOME = $$(ARROW_HOME)
}
!isEmpty(ARROW_HOME) {
    exists ($${ARROW_HOME}/include) {
        DEFINES += HAS_PARQUET
        QMAKE_CXXFLAGS += -std=c++20
        QMAKE_CXXFLAGS += -isystem $${ARROW_HOME}/include
        LIBS += -L$${ARROW_HOME}/lib64
        LIBS += -larrow -lparquet
        QMAKE_LFLAGS += -Wl,-rpath,$${ARROW_HOME}/lib64
    } else {
        error(ARROW_HOME=$${ARROW_HOME} does not contain an include directory)
    }
}

QMAKE_CXXFLAGS += -Wno-deprecated-enum-enum-conversion

macx {
    QMAKE_CXXFLAGS += -Wno-implicit-function-declaration
}

#CONFIG += debug
