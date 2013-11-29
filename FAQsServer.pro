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
    DAO.cpp \
    SimilarityComparer.cpp \
    Main.cpp \
    Template.cpp \
    SnippetCreator.cpp \
    Settings.cpp
HEADERS = \
    Server.h \
    DAO.h \
    SimilarityComparer.h \
    Template.h \
    SnippetCreator.h \
    Settings.h
