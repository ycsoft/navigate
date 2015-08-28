TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    ../src/navigate.cpp \
    ../src/navigatelib.cpp

HEADERS += \
    ../src/defines.h \
    ../src/navigate.h \
    ../src/navigatelib.h

