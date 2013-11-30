#-------------------------------------------------
#
# Project created by QtCreator 2013-03-10T16:47:12
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = RappelzPlayerCount
CONFIG   += console
CONFIG   -= app_bundle

unix {
    QMAKE_CXXFLAGS += -std=c++11
}

INCLUDEPATH += ../RappelzLib/

LIBS += -L../RappelzLib/bin -lRappelzLib

TEMPLATE = app

OBJECTS_DIR = objs
DESTDIR = bin
MOC_DIR = objs
RCC_DIR = objs
UI_DIR = ui

SOURCES += main.cpp \
    PlayerCountMonitor.cpp

HEADERS += \
    PlayerCountMonitor.h
