#ifndef STATEESTIMATOR_H
#define STATEESTIMATOR_H

#include "mahaon/imu.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <fusioncore/fusioncore.hpp>

class StateEstimator
{
public:
    struct Pose {
        double x, y, yaw;
    };
    StateEstimator(boost::asio::io_context &io_context);
    void setLaserOdom(double stamp, double x, double y, double yaw, double vx, double vy, double vz);
    void shutdown();
    void reset();
    inline void setImuWeight(float weight) { m_imuWeight = weight; }

    inline void onPositionChanged(const std::function<void (double, double, double)> &callback) {
        m_positionChangedCallback = callback;
    }

private:
    Pose m_pose;
    Pose m_laserOdom;
    double m_imuYaw;
    double m_originImuYaw;
    float m_imuWeight;
    boost::asio::steady_timer m_timer;
    IMU m_imu;
    fusioncore::FusionCore m_fusionCore;

    std::function<void (double, double, double)> m_positionChangedCallback;

    void stateEstimate(const boost::system::error_code &ec);
};

#endif // STATEESTIMATOR_H
