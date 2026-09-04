#ifndef LIDAR_H
#define LIDAR_H

#include <QObject>
#include <atomic>
#include <memory>
#include <src/CYdLidar.h>
#include <mrpt/obs/CObservation2DRangeScan.h>
#include <mrpt/maps/CSimplePointsMap.h>
#include <mrpt/obs/CObservationPointCloud.h>

#if defined(_MSC_VER)
#pragma comment(lib, "ydlidar_sdk.lib")
#endif

class Lidar : public QObject
{
    Q_OBJECT
public:
    Lidar(QObject *parent = nullptr);
    ~Lidar();
    inline float minRange() const { return m_minRange; }
    inline float maxRange() const { return m_maxRange; }
    inline bool rangeIsValid(float range) const { return range >= m_minRange && range <= m_maxRange; }

private:
    const float m_minRange = 0.12f;
    const float m_maxRange = 8.0f;
    std::thread m_thread;
    std::atomic<bool> m_running {true};
    std::atomic<bool> m_updated {false};
    std::atomic<std::shared_ptr<LaserScan>> m_scan;

    std::pair<float, bool> interpolateValue(const std::vector<LaserPoint>& data, float targetAngle);

private slots:
    void loadScan();

signals:
    // void scanChanged(std::shared_ptr<LaserScan>);
    void scanChanged(const mrpt::obs::CObservation2DRangeScan::Ptr &obs);
    void pointCloudChanged(const mrpt::obs::CObservationPointCloud::Ptr &pointCloud);
};

#endif // LIDAR_H
