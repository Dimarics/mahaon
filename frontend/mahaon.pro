include(src/src.pri)

QT += quick serialport network websockets location

CONFIG += qmltypes c++20

QML_IMPORT_PATH += $$PWD
QML_IMPORT_NAME = QmlComponents
QML_IMPORT_MAJOR_VERSION = 1
QML_IMPORT_MINOR_VERSION = 0

RESOURCES += resources \
    resources.qrc

wasm {
    source_path = $$PWD
    source_path ~= s,/,\\,g
    install_path = $$OUT_PWD
    install_path ~= s,/,\\,g
    #QMAKE_POST_LINK += $$quote(cmd /c copy /Y "$$source_path\\$${TARGET}.dll" "$$install_path\\..\\Desktop_Qt_6_7_2_MinGW_64_bit")
}

# qnx: target.path = /tmp/$$TARGET/bin
# else: unix:!android: target.path = /opt/$$TARGET/bin
# !isEmpty(target.path): INSTALLS += target
INSTALLS += target


win32: {
    INCLUDEPATH += \
        D:\Programs\OpenCV-4.12.0\include \
        # D:\Programs\boost_1_91_0

    LIBS += -LD:\Programs\OpenCV-4.12.0\bin
    # LIBS += -lopencv_core4120
    # LIBS += -lopencv_highgui4120
    # LIBS += -lopencv_imgcodecs4120
    # LIBS += -lopencv_imgproc4120
    #LIBS += -lopencv_video4120
    # LIBS += -lopencv_videoio4120
    # LIBS += -lopencv_objdetect4120
    # LIBS += -lopencv_calib3d4120
    # LIBS += -lopencv_features2d4120
    # LIBS += -lopencv_dnn4120
    # LIBS += -lws2_32
}
