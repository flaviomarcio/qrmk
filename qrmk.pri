INCLUDEPATH += $$PWD/includes

QTREFORCE_QRMK=true
DEFINES+=QTREFORCE_QRMK

include($$PWD/src/qrmk.pri)

INCLUDEPATH += $$PWD/include/

HEADERS+= \
    $$PWD/include/QRmk
