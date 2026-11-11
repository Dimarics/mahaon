#include "mahaon/laser_scan_matcher.h"
#include "Eigen/src/Eigenvalues/SelfAdjointEigenSolver.h"

LaserScanMatcher::LaserScanMatcher()
    : m_initialized(false)
{
    m_sm_params.max_angular_correction_deg = 45.0;
    m_sm_params.max_linear_correction = 0.50;
    m_sm_params.max_iterations = 10;
    m_sm_params.epsilon_xy = 1e-6;
    m_sm_params.epsilon_theta = 1e-6;
    m_sm_params.max_correspondence_dist = 0.3;
    m_sm_params.sigma = 0.010;
    m_sm_params.use_corr_tricks = 1;
    m_sm_params.restart = 0;
    m_sm_params.restart_threshold_mean_error = 0.01;
    m_sm_params.restart_dt = 1.0;
    m_sm_params.restart_dtheta = 0.1;
    m_sm_params.clustering_threshold = 0.25;
    m_sm_params.orientation_neighbourhood = 20;
    m_sm_params.use_point_to_line_distance = 1;
    m_sm_params.do_alpha_test = 0;
    m_sm_params.do_alpha_test_thresholdDeg = 20.0;
    m_sm_params.outliers_maxPerc = 0.90;
    m_sm_params.outliers_adaptive_order = 0.7;
    m_sm_params.outliers_adaptive_mult = 2.0;
    m_sm_params.do_visibility_test = 0;
    m_sm_params.do_compute_covariance = 1;
    m_sm_params.debug_verify_tricks = 0;
    m_sm_params.use_ml_weights = 0;
    m_sm_params.use_sigma_weights = 0;

    m_sm_params.laser[0] = 0.0;
    m_sm_params.laser[1] = 0.0;
    m_sm_params.laser[2] = 0.0;
}

void LaserScanMatcher::newScan(const std::shared_ptr<LaserScan> &scan) {
    if (!m_initialized) {
        m_initialized = true;
        m_sm_params.min_reading = scan->config.min_range;
        m_sm_params.max_reading = scan->config.max_range;
        laserScanToLDP(scan, m_prev_ldp);
    } else {
        LDP curr_ldp_scan;
        laserScanToLDP(scan, curr_ldp_scan);
        processScan(curr_ldp_scan, rclcpp::Time(scan->stamp));
    }
}

void LaserScanMatcher::newScan(const sensor_msgs::msg::LaserScan::SharedPtr &scan_msg) {
    if (!m_initialized) {
        m_initialized = true;
        m_sm_params.min_reading = scan_msg->range_min;
        m_sm_params.max_reading = scan_msg->range_max;
        laserScanMsgToLDP(scan_msg, m_prev_ldp);
    } else {
        LDP curr_ldp_scan;
        laserScanMsgToLDP(scan_msg, curr_ldp_scan);
        processScan(curr_ldp_scan, scan_msg->header.stamp);
    }
}

void LaserScanMatcher::processScan(LDP &curr_ldp_scan, const rclcpp::Time &stamp) {
    sm_result result;

    m_sm_params.first_guess[0] = 0.0;
    m_sm_params.first_guess[1] = 0.0;
    m_sm_params.first_guess[2] = 0.0;

    m_prev_ldp->odometry[0] = 0.0;
    m_prev_ldp->odometry[1] = 0.0;
    m_prev_ldp->odometry[2] = 0.0;

    m_prev_ldp->estimate[0] = 0.0;
    m_prev_ldp->estimate[1] = 0.0;
    m_prev_ldp->estimate[2] = 0.0;

    m_prev_ldp->true_pose[0] = 0.0;
    m_prev_ldp->true_pose[1] = 0.0;
    m_prev_ldp->true_pose[2] = 0.0;

    m_sm_params.laser_ref = m_prev_ldp;
    m_sm_params.laser_sens = curr_ldp_scan;

    sm_icp(&m_sm_params, &result);

    if (result.valid) {
        static double gx = 0, gy = 0, gyaw = 0;
        double x = result.x[0];
        double y = result.x[1];
        double yaw = result.x[2];
        gx += x * cos(gyaw) - y * sin(gyaw);
        gy += x * sin(gyaw) + y * cos(gyaw);
        gyaw += yaw;
        m_odomChangedCallback(stamp, gx, gy, gyaw);

        Eigen::Matrix2d sigma;
        sigma(0,0) = gsl_matrix_get(result.cov_x_m, 0, 0);
        sigma(0,1) = gsl_matrix_get(result.cov_x_m, 0, 1);
        sigma(1,0) = gsl_matrix_get(result.cov_x_m, 1, 0);
        sigma(1,1) = gsl_matrix_get(result.cov_x_m, 1, 1);

        // Разложение по собственным значениям
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix2d> solver(sigma);
        Eigen::Vector2d eigenvalues = solver.eigenvalues();   // возрастающий порядок: [λ2, λ1]
        Eigen::Matrix2d eigenvectors = solver.eigenvectors(); // столбцы — собственные векторы

        double lambda_max = eigenvalues(1); // λ1 (большее)
        double lambda_min = eigenvalues(0); // λ2 (меньшее)
        Eigen::Vector2d v_max = eigenvectors.col(1); // направление максимальной дисперсии

        // Вычисление параметров эллипса (уровень 2σ)
        double k = 2.0;
        double a = k * std::sqrt(std::max(lambda_max, 0.0));
        double b = k * std::sqrt(std::max(lambda_min, 0.0));
        double phi = std::atan2(v_max.y(), v_max.x());

        m_covarianceEllipseChangedCallback(a, b, phi);
    }
    m_prev_ldp = curr_ldp_scan;
}

void LaserScanMatcher::laserScanToLDP(const std::shared_ptr<LaserScan> &scan, LDP &ldp) {
    unsigned int n = scan->size;
    ldp = ld_alloc_new(n);

    for (unsigned int i = 0; i < n; i++) {
        double r = scan->points[i].range;
        if (r > scan->config.min_range && r < scan->config.max_range) {
            ldp->valid[i] = true;
            ldp->readings[i] = r;
        } else {
            ldp->valid[i] = false;
            ldp->readings[i] = -1;
        }
        ldp->theta[i]    = scan->points[i].angle;
        ldp->cluster[i]  = -1;
    }

    ldp->min_theta = ldp->theta[0];
    ldp->max_theta = ldp->theta[n-1];

    ldp->odometry[0] = 0.0;
    ldp->odometry[1] = 0.0;
    ldp->odometry[2] = 0.0;

    ldp->true_pose[0] = 0.0;
    ldp->true_pose[1] = 0.0;
    ldp->true_pose[2] = 0.0;
}

void LaserScanMatcher::LaserScanMatcher::laserScanMsgToLDP(const sensor_msgs::msg::LaserScan::SharedPtr &scan_msg, LDP &ldp)
{
    unsigned int n = scan_msg->ranges.size();
    ldp = ld_alloc_new(n);

    for (unsigned int i = 0; i < n; i++) {
        double r = scan_msg->ranges[i];
        if (r > scan_msg->range_min && r < scan_msg->range_max) {
            ldp->valid[i] = true;
            ldp->readings[i] = r;
        } else {
            ldp->valid[i] = false;
            ldp->readings[i] = -1;
        }
        ldp->theta[i]    = scan_msg->angle_min + i * scan_msg->angle_increment;
        ldp->cluster[i]  = -1;
    }

    ldp->min_theta = ldp->theta[0];
    ldp->max_theta = ldp->theta[n-1];

    ldp->odometry[0] = 0.0;
    ldp->odometry[1] = 0.0;
    ldp->odometry[2] = 0.0;

    ldp->true_pose[0] = 0.0;
    ldp->true_pose[1] = 0.0;
    ldp->true_pose[2] = 0.0;
}
