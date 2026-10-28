#ifndef COSTMAPWIDGET_H
#define COSTMAPWIDGET_H

#include <QWidget>
// #include "point.h"
#include "costmap.h"
#include "gridmap.h"
#include "navfn.hpp"
#include "theta_star/theta_star.hpp"

class CostmapWidget : public QWidget
{
    Q_OBJECT
public:
    CostmapWidget(QWidget *parent = nullptr);

private:
    PointF m_start;
    PointF m_goal;
    std::shared_ptr<GridMap> m_gridMap;
    CostMap m_costMap;
    ThetaStar m_thetaStarPlanner;
    NavFn m_navfnPlanner;
    std::vector<PointF> m_path;

    void calculateThetaStar();

    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // COSTMAPWIDGET_H
