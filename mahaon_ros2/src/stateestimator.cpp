#include "mahaon/stateestimator.h"
#include <iostream>

StateEstimator::StateEstimator(boost::asio::io_context &io_context)
    : m_pose {0, 0, 0}
    , m_timer(io_context, std::chrono::milliseconds(20))
    , m_imu(io_context)
{
    m_timer.async_wait(std::bind_front(&StateEstimator::stateEstimate, this));

    // IMU
    m_imu.onYawChanged([this](double yaw) {
        static uint8_t count = false;
        if (count < 8) {
            ++count;
            m_originImuYaw = m_imuYaw;
            std::cout << m_originImuYaw << std::endl;
        }
        m_imuYaw = yaw;
    });
}

void StateEstimator::setLaserOdom(double x, double y, double yaw) {
    m_laserOdom.x = x;
    m_laserOdom.y = y;
    m_laserOdom.yaw = yaw;
}

void StateEstimator::shutdown() {
    m_timer.cancel();
    m_imu.shutdown();
}

void StateEstimator::reset() {
    m_originImuYaw = m_imuYaw;
}

void StateEstimator::stateEstimate(const boost::system::error_code &error_code) {
    if (error_code) return;
    // std::cout << m_originImuYaw << std::endl;

    m_pose.x = m_laserOdom.x;
    m_pose.y = m_laserOdom.y;
    // m_pose.yaw = m_laserOdom.yaw;
    // m_pose.yaw = m_imuYaw - m_originImuYaw;
    m_pose.yaw = 0.6 * m_laserOdom.yaw + 0.4 * (m_imuYaw - m_originImuYaw);

    // std::cout << m_laserOdom.yaw << ' ' << (m_imuYaw - m_originImuYaw) << std::endl;

    m_positionChangedCallback(m_pose.x, m_pose.y, m_pose.yaw);

    m_timer.expires_at(m_timer.expiry() + std::chrono::milliseconds(20));
    m_timer.async_wait(std::bind_front(&StateEstimator::stateEstimate, this));
}
