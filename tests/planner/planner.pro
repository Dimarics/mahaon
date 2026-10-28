QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    costmap.cpp \
    costmapwidget.cpp \
    gridmap.cpp \
    main.cpp \
    mainwindow.cpp \
    navfn.cpp \
    theta_star/theta_star.cpp

HEADERS += \
    costmap.h \
    costmapwidget.h \
    gridmap.h \
    mainwindow.h \
    navfn.hpp \
    point.h \
    theta_star/theta_star.hpp

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
