#-------------------------------------------------
#
# Project created by QtCreator 2013-03-10T16:47:12
#
#-------------------------------------------------

QT       -= gui core

TARGET = RappelzPlayerCount
CONFIG   += console
CONFIG   -= app_bundle

unix {
    QMAKE_CXXFLAGS += -std=c++11
}

INCLUDEPATH += ../RappelzLib/shared-lib ../libuv/include

LIBS += -L../RappelzLib/build-linux-amd64-bin/ -lRappelzLib -L../libuv/project-linux/build-linux-amd64-bin/ -luv

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
