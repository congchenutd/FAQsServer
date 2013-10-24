cache()
TARGET = FAQsServer

QT += network sql
QT -= gui

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/include

win32 {
    debug: LIBS += -L$$PWD/lib/ -lqhttpserverd
    else:  LIBS += -L$$PWD/lib/ -lqhttpserver
} else {
    LIBS += -L$$PWD/lib -lqhttpserver
}

SOURCES = \
    Server.cpp \
    DAO.cpp
HEADERS = \
    Server.h \
    DAO.h
