include($$PWD/../qrmk.pri)

#INCLUDEPATH+=$$PWD

CONFIG += console
CONFIG -= debug_and_release
CONFIG += testcase
LIBS += -L/usr/local/lib -lgmock
LIBS += -L/usr/local/lib -lgtest

Q_RMK_TEST=true
QMAKE_CXXFLAGS += -DQ_RMK_TEST=\\\"$$Q_RMK_TEST\\\"

HEADERS += \
    $$PWD/qrmk_test.h \
    $$PWD/qrmk_test_unit.h \
    $$PWD/qrmk_test_functional.h

SOURCES += \
    $$PWD/qrmk_functional_test.cpp