INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/BNO085.h \
    $$PWD/I2CDevice.h \
    $$PWD/sh2.h \
    $$PWD/sh2_SensorValue.h \
    $$PWD/sh2_err.h \
    $$PWD/sh2_hal.h \
    $$PWD/sh2_util.h \
    $$PWD/shtp.h

SOURCES += \
    $$PWD/BNO085.cpp \
    $$PWD/I2CDevice.cpp \
    $$PWD/sh2.c \
    $$PWD/sh2_SensorValue.c \
    $$PWD/sh2_util.c \
    $$PWD/shtp.c


