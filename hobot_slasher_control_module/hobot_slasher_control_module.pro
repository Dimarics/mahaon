include(src/src.pri)

QT += serialport websockets
CONFIG += c++20
# INSTALLS += target

QML_IMPORT_PATH += $$PWD
QML_IMPORT_NAME = QmlComponents
QML_IMPORT_MAJOR_VERSION = 1
QML_IMPORT_MINOR_VERSION = 0
