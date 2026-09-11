#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <QList>
#include <mrpt/maps/COccupancyGridMap2D.h>
#include <mrpt/nav/tpspace/CPTG_DiffDrive_CCS.h>
#include <mrpt/nav/reactive/CReactiveNavigationSystem.h>

class Navigator
{
public:
    Navigator(mrpt::nav::CRobot2NavInterface &robot_interface);
    inline void setMap(const mrpt::maps::COccupancyGridMap2D::Ptr &map) { m_map = map; }
    void addWaypoint(double x, double y, double phi = 0.0, bool relative = false);
    void addWaypoints(const QList<mrpt::math::TPoint2D> &points);
    void update();
    inline void suspend() { m_navigator.suspend(); }
    inline void resume() { m_navigator.resume(); }
    inline void cancel() { m_navigator.cancel(); }

private:
    double m_targetX = 0;
    double m_targetY = 0;
    double m_kpAngle = 1.2;
    double m_maxLinear = 0.5;
    double m_maxAngular = 1.0;
    double m_arrRadius = 0.15;
    // double m_shapeRadius = 0.15;
    mrpt::maps::COccupancyGridMap2D::Ptr m_map;
    mrpt::nav::CReactiveNavigationSystem m_navigator;
    mrpt::nav::CRobot2NavInterface &m_robotInterface;
    mrpt::nav::CAbstractPTGBasedReactive::TNavigationParamsPTG m_navigationParamsPTG;
    void moveToPoint();
};

#endif // NAVIGATOR_H
