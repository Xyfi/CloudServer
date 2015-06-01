#-------------------------------------------------
#
# Project created by QtCreator 2015-05-23T19:09:23
#
#-------------------------------------------------

QT       += core network sql

QT       -= gui

TARGET = datatransferserver

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    server.cpp \
    connectionhandler.cpp \
    clientrequestparser.cpp \
    serverresponsebuilder.cpp \
    serverdatabase.cpp

HEADERS += \
    server.h \
    connectionhandler.h \
    clientrequestparser.h \
    serverresponsebuilder.h \
    serverdatabase.h
