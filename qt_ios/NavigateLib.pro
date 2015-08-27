#-------------------------------------------------
#
# Project created by QtCreator 2015-07-06T11:55:15
#
#-------------------------------------------------

QT       -= core gui

TARGET = NavigateLib
TEMPLATE = lib

DEFINES += NAVIGATELIB_LIBRARY


#TEMPLATE = app
#CONFIG += console
#CONFIG -= app_bundle
#CONFIG -= qt


SOURCES += navigatelib.cpp \
    ../src/navigate.cpp

HEADERS += navigatelib.h \
    ../src/defines.h \
    ../src/navigate.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}