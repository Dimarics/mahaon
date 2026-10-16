include(BNO085/BNO085.pri)
include(LDS/LDS.pri)

INCLUDEPATH += $$PWD

HEADERS += \
    # $$PWD/BNO085.h \
    $$PWD/GPS_ATGM336.h \
    $$PWD/TinyGPS++.h \
    $$PWD/TwoWire.h \
    $$PWD/controller.h \
    $$PWD/i2c.h \
    $$PWD/lidar.h \
    $$PWD/mapper.h \
    # $$PWD/rosscansubscriber.h
    $$PWD/navigator.h

SOURCES += \
    # $$PWD/BNO085.cpp \
    $$PWD/GPS_ATGM336.cpp \
    $$PWD/TwoWire.cpp \
    $$PWD/controller.cpp \
    $$PWD/i2c.cpp \
    $$PWD/lidar.cpp \
    $$PWD/main.cpp \
    $$PWD/mapper.cpp \
    $$PWD/TinyGPS++.cpp \
    # $$PWD/rosscansubscriber.cpp
    $$PWD/navigator.cpp

MRPT_INCLUDE_PATH=/usr/include

win32: {
    MRPT_INCLUDE_PATH=D:/Programs/mola/install/mrpt/include
    INCLUDEPATH += \
        D:/Programs/boost_1_91_0 \
        D:/Programs/mola/install/mrpt/3rdparty/nanoflann/include \
        D:/Programs/mola/install/mrpt/libs/math/include \
        D:/Programs/mola/install/gtsam/include \
        D:/Programs/mola/install/mp2p_icp/include \
        D:/Programs/mola/install/mola_kernel/include \
        D:/Programs/mola/install/mola_mapper_2d/include \
        D:/Programs/YDLidar-SDK/build/install/include \

    LIBS += -LD:/Programs/YDLidar-SDK/build/install/lib -lydlidar_sdk -lws2_32 -lsetupapi
} unix: {
    MRPT_INCLUDE_PATH=/home/mes/mola/install/mrpt/include
    INCLUDEPATH += \
        /usr/include/eigen3 \
        # /home/mes/mola/install/mp2p_icp/include \
        # /home/mes/mola/install/mola_kernel/include \
        # /home/mes/mola/install/mola_mapper_2d/include \
        /home/mes/mola/install/YDLidar-SDK/include \
        /usr/include/suitesparse \
        # /opt/ros/jazzy/include/rclcpp \
        # /opt/ros/jazzy/include/sensor_msgs/sensor_msgs/msg

    LIBS += \
        -L/home/mes/mola/install/YDLidar-SDK/lib -lydlidar_sdk \
        -L/home/mes/hobot_slasher_control_module/build/lib \
        -lmrpt-bayes \
        -lmrpt-config \
        -lmrpt-containers \
        -lmrpt-core \
        -lmrpt-expr \
        -lmrpt-graphs \
        -lmrpt-graphslam \
        -lmrpt-gui \
        -lmrpt-img \
        -lmrpt-io \
        -lmrpt-kinematics \
        -lmrpt-maps \
        -lmrpt-math \
        -lmrpt-nav \
        -lmrpt-nanogui \
        -lmrpt-obs \
        -lmrpt-opengl \
        -lmrpt-poses \
        -lmrpt-random \
        -lmrpt-rtti \
        -lmrpt-serialization \
        -lmrpt-slam \
        -lmrpt-system \
        -lmrpt-tfest \
        -lmrpt-vision \
        # -lgtsam -lgtsam2mrpt_serial \
        # -lmp2p_icp -lmp2p_icp_filters -lmp2p_icp_map -lmp2p_icp_common \
        # -lmola_kernel -lmola_imu_preintegration -lmola_mapper_2d
}

INCLUDEPATH += \
    $$MRPT_INCLUDE_PATH \
    $$MRPT_INCLUDE_PATH/mrpt/bayes/include \
    $$MRPT_INCLUDE_PATH/mrpt/config/include \
    $$MRPT_INCLUDE_PATH/mrpt/containers/include \
    $$MRPT_INCLUDE_PATH/mrpt/core/include \
    $$MRPT_INCLUDE_PATH/mrpt/expr/include \
    $$MRPT_INCLUDE_PATH/mrpt/graphs/include \
    $$MRPT_INCLUDE_PATH/mrpt/graphslam/include \
    $$MRPT_INCLUDE_PATH/mrpt/gui/include \
    $$MRPT_INCLUDE_PATH/mrpt/img/include \
    $$MRPT_INCLUDE_PATH/mrpt/io/include \
    $$MRPT_INCLUDE_PATH/mrpt/kinematics/include \
    $$MRPT_INCLUDE_PATH/mrpt/maps/include \
    $$MRPT_INCLUDE_PATH/mrpt/math/include \
    $$MRPT_INCLUDE_PATH/mrpt/nav/include \
    $$MRPT_INCLUDE_PATH/mrpt/obs/include \
    $$MRPT_INCLUDE_PATH/mrpt/opengl/include \
    $$MRPT_INCLUDE_PATH/mrpt/poses/include \
    $$MRPT_INCLUDE_PATH/mrpt/rtti/include \
    $$MRPT_INCLUDE_PATH/mrpt/serialization/include \
    $$MRPT_INCLUDE_PATH/mrpt/slam/include \
    $$MRPT_INCLUDE_PATH/mrpt/system/include \
    $$MRPT_INCLUDE_PATH/mrpt/tfest/include \
    $$MRPT_INCLUDE_PATH/mrpt/topography/include \
    $$MRPT_INCLUDE_PATH/mrpt/typemeta/include
