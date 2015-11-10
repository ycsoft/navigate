TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    ../../src/common.cpp \
    ../../src/navigate.cpp \
    ../../src/paka_api.cpp \
    ../../src/guidance.cpp \
    ../../src/location_master.cpp \
    ../../src/rssi_location.cpp


include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    ../../src/common.h \
    ../../src/navigate.h \
    ../../src/navigate_defines.h \
    ../../src/paka_api.h \
    ../../src/guidance.h \
    ../../src/location_defines.h \
    ../../src/location_master.h \
    ../../src/rssi_location.h \
    ../../src/processroadpoint.hpp \
    ../../src/navexception.hpp

