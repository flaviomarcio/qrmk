QT += core
QT += gui
QT += printsupport

QTREFORCE_QRMK=true
DEFINES+=QTREFORCE_QRMK

HEADERS += \
    $$PWD/private/p_qrmk_maker.h \
    $$PWD/qrmk.h \
    $$PWD/qrmk_global.h \
    $$PWD/qrmk_maker.h \
    $$PWD/qrmk_header.h \
    $$PWD/qrmk_headers.h \
    $$PWD/qrmk_signature.h \
    $$PWD/qrmk_signatures.h

SOURCES += \
    $$PWD/private/p_qrmk_maker.cpp \
    $$PWD/qrmk_maker.cpp \
    $$PWD/qrmk_header.cpp \
    $$PWD/qrmk_headers.cpp \
    $$PWD/qrmk_signature.cpp \
    $$PWD/qrmk_signatures.cpp
