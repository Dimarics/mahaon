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
    mrpt::maps::COccupancyGridMap2D::Ptr m_map;
    mrpt::nav::CReactiveNavigationSystem m_navigator;
};

#endif // NAVIGATOR_H
