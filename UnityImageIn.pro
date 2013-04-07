#-------------------------------------------------
#
# Project created by QtCreator 2012-10-03T14:00:57
#
#-------------------------------------------------

QT       += core

QT       += gui

TARGET = UnityImageIn
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
QT += widgets

SOURCES += main.cpp

HEADERS += \
    pvrtc_dll.h

LIBS += -L../UnityImageOut -lpvrtc
