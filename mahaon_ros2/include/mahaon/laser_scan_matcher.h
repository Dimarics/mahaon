#ifndef LASER_SCAN_MATCHER_H
#define LASER_SCAN_MATCHER_H

#include <csm/csm.h>
#include <mahaon/rosbridge.h>
#include <src/CYdLidar.h>

class LaserScanMatcher
{
public:
    LaserScanMatcher();

    void newScan(const std::shared_ptr<LaserScan> &scan);
    void newScan(const sensor_msgs::msg::LaserScan::SharedPtr &scan_msg);

    inline void onOdomChanged(const std::function<void (const rclcpp::Time&, double, double, double)> &callback) {
        m_odomChangedCallback = callback;
    }
    inline void onCovarianceEllipseChanged(const std::function<void (double, double, double)> &callback) {
        m_covarianceEllipseChangedCallback = callback;
    }

private:
    bool m_initialized;
    LDP m_prev_ldp;
    sm_params m_sm_params;

    void processScan(LDP& curr_ldp_scan, const rclcpp::Time &stamp);
    void laserScanToLDP(const std::shared_ptr<LaserScan> &scan, LDP& ldp);
    void laserScanMsgToLDP(const sensor_msgs::msg::LaserScan::SharedPtr &scan_msg, LDP& ldp);

    std::function<void (const rclcpp::Time&, double, double, double)> m_odomChangedCallback;
    std::function<void (double, double, double)> m_covarianceEllipseChangedCallback;
};

#endif // LASER_SCAN_MATCHER_H
