#include "mahaon/imu.h"
#include <tf2/LinearMath/Matrix3x3.hpp>
#include <iostream>

IMU::IMU(boost::asio::io_context &io_context)
    : m_connected(false)
    , m_timer(io_context, std::chrono::milliseconds(20))
{
    m_timer.async_wait(std::bind_front(&IMU::poll, this));
}

void IMU::shutdown() {
    m_timer.cancel();
}

void IMU::poll(const boost::system::error_code &error_code) {
    if (error_code) return;
    if (m_connected) {
        auto stamp = std::chrono::steady_clock::now();
        if (m_bno085.getSensorEvent(&m_sensorValue)) {
            switch (m_sensorValue.sensorId) {
            // case SH2_ROTATION_VECTOR: {
            //     // std::cout << "SH2_ROTATION_VECTOR" << std::endl;
            //     tf2::Quaternion q(
            //                 m_sensorValue.un.rotationVector.i,
            //                 m_sensorValue.un.rotationVector.j,
            //                 m_sensorValue.un.rotationVector.k,
            //                 m_sensorValue.un.rotationVector.real
            //                 );
            //     tf2::Matrix3x3 mat(q);
            //     double roll, pitch, yaw;
            //     mat.getRPY(roll, pitch, yaw);
            //     // std::cout << "yaw: " << yaw << std::endl;
            //     m_yawChangedCallback(yaw);
            //     break;
            // }
            case SH2_GAME_ROTATION_VECTOR: {
                tf2::Quaternion q(
                            m_sensorValue.un.rotationVector.i,
                            m_sensorValue.un.rotationVector.j,
                            m_sensorValue.un.rotationVector.k,
                            m_sensorValue.un.rotationVector.real
                            );
                tf2::Matrix3x3 mat(q);
                double roll, pitch, yaw;
                mat.getRPY(roll, pitch, yaw);
                m_yawChangedCallback(yaw);
                break;
            }
            // case SH2_GEOMAGNETIC_ROTATION_VECTOR: {
            //     tf2::Quaternion q(
            //                 m_sensorValue.un.geoMagRotationVector.i,
            //                 m_sensorValue.un.geoMagRotationVector.j,
            //                 m_sensorValue.un.geoMagRotationVector.k,
            //                 m_sensorValue.un.geoMagRotationVector.real
            //                 );
            //     tf2::Matrix3x3 mat(q);
            //     double roll, pitch, yaw;
            //     mat.getRPY(roll, pitch, yaw);
            //     m_yawChangedCallback(yaw);
            //     break;
            // }
            case SH2_LINEAR_ACCELERATION:
                // std::cout << "SH2_LINEAR_ACCELERATION" << std::endl;
                // hsp.sendImu(sensorValue.un.rotationVector.real,
                //             sensorValue.un.rotationVector.i,
                //             sensorValue.un.rotationVector.j,
                //             sensorValue.un.rotationVector.k,
                //             sensorValue.un.linearAcceleration.x,
                //             sensorValue.un.linearAcceleration.y,
                //             sensorValue.un.linearAcceleration.z);
                break;
            }
        }
        std::cout << "time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - stamp) << std::endl;
        m_timer.expires_at(m_timer.expiry() + std::chrono::milliseconds(20));
    } else {
        if (m_bno085.begin_I2C(&m_wire, 0x4B)) {
            m_connected = true;
            // m_bno085.enableReport(SH2_MAGNETIC_FIELD_CALIBRATED, 50000);
            // m_bno085.enableReport(SH2_ROTATION_VECTOR, 50000);
            m_bno085.enableReport(SH2_GAME_ROTATION_VECTOR, 50000);
            // m_bno085.enableReport(SH2_GEOMAGNETIC_ROTATION_VECTOR, 50000);
            m_bno085.enableReport(SH2_LINEAR_ACCELERATION, 50000);
            std::cout << "BNO connected" << std::endl;
        } else {
            std::cout << "BNO reconnect..." << std::endl;
        }
        m_timer.expires_at(m_timer.expiry() + std::chrono::seconds(1));
    }
    m_timer.async_wait(std::bind_front(&IMU::poll, this));
}
