#include "mahaon/stateestimator.h"
#include "mahaon/rosbridge.h"
#include <iostream>

StateEstimator::StateEstimator(boost::asio::io_context &io_context)
    : m_pose {0, 0, 0}
    , m_imuWeight(0.f)
    , m_timer(io_context, std::chrono::milliseconds(20))
    , m_imu(io_context)
{
    m_timer.async_wait(std::bind_front(&StateEstimator::stateEstimate, this));

    // fusioncore
    fusioncore::FusionCoreConfig config;
    // config.max_measurement_delay  = get_parameter("max_measurement_delay").as_double();
    config.outlier_rejection      = true;
    // config.outlier_threshold_gnss = 16.27;
    // config.gnss_max_speed         = get_parameter("gnss.max_speed").as_double();
    // config.gnss_max_speed_margin  = get_parameter("gnss.max_speed_margin").as_double();
    // config.gnss_max_speed_sigma_k = get_parameter("gnss.max_speed_sigma_k").as_double();
    // config.gnss_max_speed_drift_k = get_parameter("gnss.max_speed_drift_k").as_double();
    config.outlier_threshold_imu   = 13.82; //15.09;
    config.outlier_threshold_enc   = 11.34;
    config.outlier_threshold_hdg   = 10.83;
    config.outlier_threshold_vslam = 22.46;

    config.adaptive_imu               = false;
    config.adaptive_encoder           = false;
    config.adaptive_gnss              = false;
    config.adaptive_ground_constraint = true;
    config.adaptive_window            = 50;
    config.adaptive_alpha             = 0.01;

    // imu
    config.imu.gyro_noise_x = 0.005;
    config.imu.gyro_noise_y = config.imu.gyro_noise_x;
    config.imu.gyro_noise_z = config.imu.gyro_noise_x;
    config.imu.accel_noise_x = 0.1;
    config.imu_has_magnetometer = false;
    config.imu.accel_noise_y = config.imu.accel_noise_x;
    config.imu.accel_noise_z = config.imu.accel_noise_x;
    config.imu.lever_arm.x = 0.0;
    config.imu.lever_arm.y = 0.0;
    config.imu.lever_arm.z = 0.0;

    // lidar
    config.encoder.vel_noise_x = 0.02; // vel_noise
    config.encoder.vel_noise_y = config.encoder.vel_noise_x;
    config.encoder.vel_noise_wz = 0.005; // yaw_noise

    // ukf
    config.ukf.q_position = 0.01;
    config.ukf.q_orientation = 1.0e-9;
    config.ukf.q_velocity = 0.1;
    config.ukf.q_angular_vel = 0.1;
    config.ukf.q_acceleration = 1.0;
    config.ukf.q_gyro_bias = 1.0e-5;
    config.ukf.q_accel_bias = 1.0e-5;
    config.ukf.q_encoder_wz_bias = 1.0e-7;

    // input.gnss_crs: "EPSG:4326"
    // output.crs: "EPSG:4978"
    // output.convert_to_enu_at_reference: true
    // config.reference_use_first_fix = false
    // reference.x: 0.0
    // reference.y: 0.0
    // reference.z: 0.0

    // bool reference_use_first_fix_ = false;
    // if (!reference_use_first_fix_) {
    //     gnss_ref_ecef_.x = 0;
    //     gnss_ref_ecef_.y = 0;
    //     gnss_ref_ecef_.z = 0;
    //     output_to_gnss(gnss_ref_ecef_, gnss_ref_lla_);
    //     gnss_ref_set_ = true;
    // } else {
    //     // use GPS
    // }
    double stamp = ROSbridge::timestamp().seconds();
    m_fusionCore = fusioncore::FusionCore(config);
    m_fusionCore.init(fusioncore::State(), stamp);
    m_fusionCore.update_imu(stamp, 0, 0, 0, 0, 0, 9.81);
    m_fusionCore.update_encoder(stamp, 0, 0, 0);

    // IMU
    m_imu.onRawChanged([this](double wx, double wy, double wz, double ax, double ay, double az) {
        std::cout << "[state_estimator] " << wx << ' ' << wy << ' ' << wz << std::endl;
        // m_fusionCore.update_imu(ROSbridge::timestamp().seconds(), 0, 0, 0, 0, 0, 9.81);
        // m_fusionCore.update_imu(ROSbridge::timestamp().seconds(), wx, wy, wz, ax, ay, az);
        // RCLCPP_INFO(ROSbridge::logger(), "IMU stamp: %.3f", ROSbridge::timestamp().seconds());
    });
    m_imu.onRotationChanged([this](double roll, double pitch, double yaw) {
        static uint8_t count = false;
        if (count < 8) {
            ++count;
            m_originImuYaw = m_imuYaw;
            std::cout << m_originImuYaw << std::endl;
        }
        m_imuYaw = yaw;
        // m_fusionCore.update_imu_orientation(ROSbridge::timestamp().seconds(), roll, pitch, yaw);
    });
}

void StateEstimator::setLaserOdom(double stamp, double x, double y, double yaw, double vx, double vy, double vz) {
    m_laserOdom.x = x;
    m_laserOdom.y = y;
    m_laserOdom.yaw = yaw;
    // m_fusionCore.update_encoder(stamp, vx, vy, vz);
    // RCLCPP_INFO(ROSbridge::logger(), "odom: %.2f, %.2f, %.2f", vx, vy, vz);
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
    m_pose.x = m_laserOdom.x;
    m_pose.y = m_laserOdom.y;
    // m_pose.yaw = m_laserOdom.yaw;
    // m_pose.yaw = m_imuYaw - m_originImuYaw;
    m_pose.yaw = (1.f - m_imuWeight) * m_laserOdom.yaw + m_imuWeight * (m_imuYaw - m_originImuYaw);
    m_positionChangedCallback(m_pose.x, m_pose.y, m_pose.yaw);


    // fusion core
    // auto state = m_fusionCore.get_state();
    // RCLCPP_INFO(ROSbridge::logger(), "pos: %.2f, %.2f, %.2f", state.position_x(), state.position_y(), state.yaw() / M_PI * 180.0);

    m_timer.expires_at(m_timer.expiry() + std::chrono::milliseconds(100));
    m_timer.async_wait(std::bind_front(&StateEstimator::stateEstimate, this));
}
