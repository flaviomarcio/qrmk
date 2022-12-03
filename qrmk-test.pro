QT += testlib

CONFIG += testcase
CONFIG += console
CONFIG += silent
CONFIG -= debug_and_release

TEMPLATE = app
TARGET = QRmkTest

INCLUDEPATH+=$$PWD/../src

QRMK_TEST_MODE=true
QMAKE_CXXFLAGS += -DQRMK_TEST_MODE=\\\"$$QRMK_TEST_MODE\\\"

LIBS += -lgmock
LIBS += -lgtest

include($$PWD/src/qrmk.pri)
include($$PWD/src/qrmk-test.pri)

