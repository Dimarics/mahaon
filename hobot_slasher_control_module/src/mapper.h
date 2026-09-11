#ifndef MAPPER_H
#define MAPPER_H

#include <QObject>
#include <QPoint>
// #include <mola_mapper_2d/Mapper2D.h>
#include <mrpt/obs/CObservation2DRangeScan.h>
#include <mrpt/obs/CObservationPointCloud.h>
#include <mrpt/poses/CPose2D.h>
#include <mrpt/maps/COccupancyGridMap2D.h>
#include <mrpt/maps/CSimplePointsMap.h>
#include <mrpt/slam/CMetricMapBuilderICP.h>
#include <mrpt/slam/CMetricMapBuilderRBPF.h>
// #include <mrpt/graphslam/CGraphSlamEngine.h>
#include <mrpt/slam/CMonteCarloLocalization2D.h>

class Mapper : public QObject
{
    Q_OBJECT
public:
    Mapper(QObject *parent = nullptr);
    ~Mapper();
    void startBuildingMap();
    void finishBuildingMap();

private:
    bool m_mclInit;
    bool m_buildingMap;
    QPoint m_currentCellPos;

    // mrpt::obs::CObservationPointCloud::Ptr m_currentObs;
    mrpt::obs::CObservation2DRangeScan::Ptr m_currentObs;

    // mola::Mapper2D m_mapper;
    // mrpt::maps::COccupancyGridMap2D m_gridMap;
    // mrpt::maps::TSetOfMetricMapInitializers mapInit;
    mrpt::slam::CMetricMapBuilderICP m_mapBuilderICP;
    // mrpt::slam::CMetricMapBuilderRBPF  m_mapBuilderRBPF;
    // mrpt::slam::CMonteCarloLocalization2D m_MCL;
    mrpt::maps::COccupancyGridMap2D::Ptr m_gridMap;

    // mrpt::graphslam::CGraphSlamEngine<mrpt::graphs::CNetworkOfPoses2DInf> *m_graphSlamEngine;

public slots:
    void processScan(const mrpt::obs::CObservation2DRangeScan::Ptr &obs);
    // void processScan(const mrpt::obs::CObservationPointCloud::Ptr &obs);

signals:
    void positionChanged(double x, double y, double yaw);
    void cellPositionChanged(const QPoint &cellPos);
    void mapChanged(const mrpt::maps::COccupancyGridMap2D::Ptr &gridMap);
    void mapUpdated(const QByteArray &map);
};

#endif // MAPPER_H
