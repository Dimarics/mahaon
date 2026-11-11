#include "mahaon/imu.h"
#include "bno085.h"
#include <boost/asio/post.hpp>
#include <tf2/LinearMath/Matrix3x3.hpp>
#include <iostream>

IMU::IMU(boost::asio::io_context &io_context) : m_running(true) {
    // m_timer.async_wait(std::bind_front(&IMU::poll, this));
    return;
    m_thread = std::thread([this, &io_context] {
        bool connected = false;
        TwoWire wire;
        Adafruit_BNO08x bno085;
        sh2_SensorValue_t sensorValue;
        while (m_running) {
            if (!connected) {
                if (bno085.begin_I2C(&wire, 0x4B)) {
                    connected = true;
                    bno085.enableReport(SH2_ACCELEROMETER, 10'000);
                    bno085.enableReport(SH2_GYROSCOPE_UNCALIBRATED, 10'000);
                    // m_bno085.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED, 50000);
                    // m_bno085.enableReport(SH2_ROTATION_VECTOR, 50000);
                    bno085.enableReport(SH2_GAME_ROTATION_VECTOR, 20'000);
                    // m_bno085.enableReport(SH2_GEOMAGNETIC_ROTATION_VECTOR, 50000);
                    // m_bno085.enableReport(SH2_LINEAR_ACCELERATION, 50000);
                    std::cout << "[IMU] BNO connected" << std::endl;
                } else {
                    std::cout << "[IMU] BNO reconnect..." << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            } else {
                // auto stamp = std::chrono::steady_clock::now();
                if (bno085.getSensorEvent(&sensorValue)) {
                    switch (sensorValue.sensorId) {
                    case SH2_GYROSCOPE_UNCALIBRATED: {
                        float wx = sensorValue.un.gyroscopeUncal.x;
                        float wy = sensorValue.un.gyroscopeUncal.y;
                        float wz = sensorValue.un.gyroscopeUncal.z;
                        float ax = sensorValue.un.accelerometer.x;
                        float ay = sensorValue.un.accelerometer.y;
                        float az = sensorValue.un.accelerometer.z;
                        boost::asio::post(io_context, [wx, wy, wz, ax, ay, az, this] {
                            m_rawChangedCallback(wx, wy, wz, ax, ay, az);
                        });
                        break;
                    }
                    case SH2_GAME_ROTATION_VECTOR: {
                        tf2::Quaternion q(
                                    sensorValue.un.rotationVector.i,
                                    sensorValue.un.rotationVector.j,
                                    sensorValue.un.rotationVector.k,
                                    sensorValue.un.rotationVector.real
                                    );
                        tf2::Matrix3x3 mat(q);
                        double roll, pitch, yaw;
                        mat.getRPY(roll, pitch, yaw);
                        boost::asio::post(io_context, [roll, pitch, yaw, this] {
                            m_rotationChangedCallback(roll, pitch, yaw);
                        });
                        break;
                    }
                    }
                }
                // std::cout << "time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - stamp) << std::endl;
            }
        }
    });
}

void IMU::shutdown() {
    m_running = false;
}
