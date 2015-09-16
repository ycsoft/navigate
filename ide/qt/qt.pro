TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    ../../src/common.cpp \
    ../../src/navigate.cpp \
    ../../src/wifi_location.cpp \
    ../../src/paka_api.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    ../../src/common.h \
    ../../src/navigate.h \
    ../../src/wifi_location.h \
    ../../src/wifi_location_defines.h \
    ../../src/paka_api.h \
    ../../src/navigate_defines.h

