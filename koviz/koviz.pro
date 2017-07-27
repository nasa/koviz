QT  += core
QT  += gui
QT  += xml

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

PRE_TARGETDEPS += $$PWD/../lib/libkoviz.a