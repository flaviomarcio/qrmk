TARGET = QRmk
TEMPLATE = lib

CONFIG+=silent

include($$PWD/../bcmath/bcmath.pri)
include($$PWD/../qstm/qstm.pri)
include($$PWD/src/qrmk.pri)

