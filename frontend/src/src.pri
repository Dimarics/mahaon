include(Components/Components.pri)

INCLUDEPATH += $$PWD
QML_IMPORT_PATH += $$PWD

HEADERS += \
    $$PWD/appglobal.h \
    $$PWD/lidarmapviewer.h \
    $$PWD/lidarviewer.h \
    $$PWD/serialportinterface.h \
    $$PWD/streamviewer.h \
    $$PWD/websocketinterface.h

SOURCES += \
    $$PWD/appglobal.cpp \
    $$PWD/lidarmapviewer.cpp \
    $$PWD/lidarviewer.cpp \
    $$PWD/main.cpp \
    $$PWD/serialportinterface.cpp \
    $$PWD/streamviewer.cpp \
    $$PWD/websocketinterface.cpp

resources.files += \
    $$PWD/main.qml \
    $$PWD/Device.qml \
    $$PWD/MapViewer.qml
