#include "costmapwidget.h"
// #include "qelapsedtimer.h"
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>

CostmapWidget::CostmapWidget(QWidget *parent)
    : QWidget{parent}
    , m_start(NAN, NAN)
    , m_gridMap(std::make_shared<GridMap>(250, 200, 10.0))
    , m_thetaStarPlanner(&m_costMap)
    , m_navfnPlanner(m_gridMap->width(), m_gridMap->height())
{
    setMinimumSize(m_gridMap->width() * m_gridMap->resolution(), m_gridMap->height() * m_gridMap->resolution());
    resize(m_gridMap->width() * m_gridMap->resolution(), m_gridMap->height() * m_gridMap->resolution());

    m_costMap.setGridMap(m_gridMap);
}

void CostmapWidget::calculateThetaStar() {
    // QElapsedTimer t;
    // t.start();
    if (std::isnan(m_start.x)) return;
    m_path.clear();
    ThetaStar planner(&m_costMap);
    planner.setStartAndGoal(m_start, m_goal);
    planner.generatePath(m_path);
    // qDebug() << "time:" << t.elapsed();
}

void CostmapWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), QColor(33, 40, 48));

    // отрисовка препятствий
    auto cost = m_costMap.cost();
    for (size_t i = 0; i < cost.size(); ++i) {
        painter.setPen(Qt::NoPen);
        if (cost.at(i) == CostMap::LETHAL_OBSTACLE) {
            painter.setBrush(QColor(127, 127, 127));
        } else {
            painter.setBrush(QColor(0, 0, 255, cost.at(i)));
        }
        qreal side = m_gridMap->resolution();
        auto pos = m_gridMap->indexToWorld(i);
        int x = pos.x;
        int y = pos.y;
        painter.drawRect(QRectF(x - side / 2.0, y - side / 2.0, side, side));
    }
    if (!std::isnan(m_start.x)) {
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::green, 2.0));
        for (size_t i = 1; i < m_path.size(); ++i) {
            painter.drawLine(QPointF(m_path[i-1].x, m_path[i-1].y), QPointF(m_path[i].x, m_path[i].y));
        }
        // navfn
        // painter.setPen(QPen(Qt::red, 2.0));
        // int path_len = m_navfnPlanner.calcPath(m_gridMap->width() * m_gridMap->width());
        // if (path_len > 0) {
        //     float* path_x = m_navfnPlanner.getPathX();
        //     float* path_y = m_navfnPlanner.getPathY();
        //     for (int i = 1; i < path_len; ++i) {
        //         auto p1 = m_gridMap->mapToWorld(path_x[i-1], path_y[i-1]);
        //         auto p2 = m_gridMap->mapToWorld(path_x[i], path_y[i]);
        //         painter.drawLine(QPointF(p1.x, p1.y), QPointF(p2.x, p2.y));
        //     }
        // }
        // -----

        painter.setPen(QPen(QColor(255, 127, 0), 2.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawLine(QPointF(m_start.x, m_start.y), QPointF(m_goal.x, m_goal.y));
    }
}

void CostmapWidget::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        m_start = {event->position().x(), event->position().y()};
        m_goal = {event->position().x(), event->position().y()};
        m_path.clear();
        // navfn
        // auto start_cell_p = m_gridMap->worldToMap(event->position().x(), event->position().y());
        // int start_cell[2] = {start_cell_p.x, start_cell_p.y};
        // m_navfnPlanner.setStart(start_cell);
        // ------
        repaint();
    }
    if (event->buttons() & Qt::RightButton) {
        m_gridMap->setCellInPos(event->position().x(), event->position().y(), 100);
        m_costMap.updateCost();
        calculateThetaStar();
        repaint();
    }
}

void CostmapWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        m_goal = {event->position().x(), event->position().y()};
        calculateThetaStar();
        // navfn
        // m_navfnPlanner.setCostmap(m_costMap.cost().data());
        // auto goal_cell_p = m_gridMap->worldToMap(event->position().x(), event->position().y());
        // int goal_cell[2] = {goal_cell_p.x, goal_cell_p.y};
        // m_navfnPlanner.setGoal(goal_cell);
        // bool found = m_navfnPlanner.calcNavFnDijkstra([](){ return false; });
        // -----
        repaint();
    }
    if (event->buttons() & Qt::RightButton) {
        m_gridMap->setCellInPos(event->position().x(), event->position().y(), 100);
        m_costMap.updateCost();
        calculateThetaStar();
        repaint();
    }
}

void CostmapWidget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        m_gridMap->setData(std::vector<int8_t>(m_gridMap->data().size(), 0));
        m_costMap.updateCost();
        m_start.x = NAN;
        repaint();
        break;
    case Qt::Key_Space:
        break;
    default:
        break;
    }
}
