TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    ../src/navigate.cpp \
    ../src/wifi_location.cpp \
    ../src/paka_api.cpp \
    ../src/common.cpp

HEADERS += \
    ../src/navigate.h \
    ../src/wifi_location.h \
    ../src/paka_api.h \
    ../src/common.h \
    ../src/navigate_defines.h \
    ../src/wifi_location_defines.h
