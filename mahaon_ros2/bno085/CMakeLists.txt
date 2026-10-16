add_library(bno085 STATIC
    src/bno085.cpp
    src/I2CDevice.cpp
    src/sh2.c
    src/sh2_SensorValue.c
    src/sh2_util.c
    src/shtp.c
    src/TwoWire.cpp
)

target_include_directories(bno085 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
