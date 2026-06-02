# =============================================================================
#
# koviz - Fast plotting for Trick simulation data
#
# =============================================================================

PROJECT_NAME = koviz
PROJECT_VERSION = 1.0.0

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
