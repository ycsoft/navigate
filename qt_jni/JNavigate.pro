#-------------------------------------------------
#
# Project created by QtCreator 2015-07-11T21:26:06
#
#-------------------------------------------------

QT       -= core gui

TARGET = JNavigate
TEMPLATE = lib

DEFINES += JNAVIGATE_LIBRARY

SOURCES += \
    ../src/navigate.cpp \
    JNavigate.cpp \
    ../src/navigatelib.cpp

HEADERS += \
    ../src/defines.h \
    ../src/navigate.h \
    navigate_Navigate.h \
    ../src/navigatelib.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += "D:\Program Files (x86)\Java\jdk1.8.0_25\include"
INCLUDEPATH += "D:\Program Files (x86)\Java\jdk1.8.0_25\include\win32"
