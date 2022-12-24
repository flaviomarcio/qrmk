QT += testlib

CONFIG += silent
CONFIG += testcase
CONFIG += console
CONFIG -= debug_and_release

TEMPLATE = app
TARGET = QRmkTest

INCLUDEPATH+=$$PWD/../src

QRMK_TEST_MODE=true
QMAKE_CXXFLAGS += -DQRMK_TEST_MODE=\\\"$$QRMK_TEST_MODE\\\"

LIBS += -lgmock
LIBS += -lgtest

include($$PWD/../qstm/qstm.pri)
include($$PWD/test/qrmk-test.pri)

