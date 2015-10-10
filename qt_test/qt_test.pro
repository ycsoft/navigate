TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    ../src/navigate.cpp \
    ../src/paka_api.cpp \
    ../src/common.cpp \
    ../src/guidance.cpp \
    ../src/location_master.cpp \
    ../src/rssi_location.cpp

HEADERS += \
    ../src/navigate.h \
    ../src/paka_api.h \
    ../src/common.h \
    ../src/navigate_defines.h \
    ../src/guidance.h \
    ../src/location_master.h \
    ../src/location_defines.h \
    ../src/rssi_location.h

