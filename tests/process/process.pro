QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    slamlauncher.cpp

HEADERS += \
    mainwindow.h \
    slamlauncher.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += D:/Programs/lib/boost_1_91_0
LIBS += -lntdll -lshell32 -ladvapi32 -luser32 -lws2_32 -lbcrypt
LIBS += -LD:/Programs/lib/boost_1_91_0/stage/lib
LIBS += -lboost_process-mgw13-mt-x64-1_91
LIBS += -lboost_filesystem-mgw13-mt-x64-1_91
