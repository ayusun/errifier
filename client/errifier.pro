#-------------------------------------------------
#
# Project created by QtCreator 2014-07-25T23:12:35
#
#-------------------------------------------------

QT       += core gui

TARGET = errifier
TEMPLATE = app

CFLAGS += -DHAVE_NETINET_IN_H
INCLUDEPATH += /usr/local/boost/include/boost-1_33_1/ \
                 /usr/local/include/thrift/ \

LIBS += "-L/usr/local/lib/"
LIBS += -lthrift -lthriftnb -lboost_thread -lthrift_c_glib -levent

SOURCES += main.cpp\
        errifierui.cpp \
    dialog.cpp \
    core/workerHandler.cpp \
    core/workercontroller_types.cpp \
    core/workercontroller_constants.cpp \
    core/workerthread.cpp

HEADERS  += errifierui.h \
    dialog.h \
    core/workerHandler.h \
    core/workercontroller_types.h \
    core/workercontroller_constants.h \
    core/workerthread.h

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += gtk+-2.0

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += glib-2.0 libnotify

CONFIG += no_keywords

FORMS    += errifierui.ui

RESOURCES     = errifier.qrc
