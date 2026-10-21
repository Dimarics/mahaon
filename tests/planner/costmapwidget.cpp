#include "costmapwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>

CostmapWidget::CostmapWidget(QWidget *parent)
    : QWidget{parent}
    , m_gridMap(std::make_shared<GridMap>(250, 200, 10.0))
    , m_thetaStarPlanner(&m_costMap)
{
    setMinimumSize(m_gridMap->width() * m_gridMap->resolution(), m_gridMap->height() * m_gridMap->resolution());
    resize(m_gridMap->width() * m_gridMap->resolution(), m_gridMap->height() * m_gridMap->resolution());

    m_costMap.setGridMap(m_gridMap);
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

    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(Qt::green, 2.0));
    painter.setBrush(Qt::NoBrush);
    for (size_t i = 1; i < m_path.size(); ++i) {
        painter.drawLine(QPointF(m_path[i-1].x, m_path[i-1].y), QPointF(m_path[i].x, m_path[i].y));
    }

    painter.setPen(QPen(QColor(255, 127, 0), 2.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(QPointF(m_start.x, m_start.y), QPointF(m_goal.x, m_goal.y));
}

void CostmapWidget::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        m_start = {event->position().x(), event->position().y()};
        m_goal = {event->position().x(), event->position().y()};
        m_path.clear();
        m_thetaStarPlanner.setStartAndGoal(m_start, m_goal);
        m_thetaStarPlanner.generatePath(m_path, []{return false;});
        repaint();
    }
    if (event->buttons() & Qt::RightButton) {
        m_gridMap->setCellInPos(event->position().x(), event->position().y(), 100);
        m_costMap.updateCost();
        m_path.clear();
        m_thetaStarPlanner.setStartAndGoal(m_start, m_goal);
        m_thetaStarPlanner.generatePath(m_path, []{return false;});
        repaint();
    }
}

void CostmapWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        m_goal = {event->position().x(), event->position().y()};
        m_path.clear();
        m_thetaStarPlanner.setStartAndGoal(m_start, m_goal);
        m_thetaStarPlanner.generatePath(m_path, []{return false;});
        repaint();
    }
    if (event->buttons() & Qt::RightButton) {
        m_gridMap->setCellInPos(event->position().x(), event->position().y(), 100);
        m_costMap.updateCost();
        m_path.clear();
        m_thetaStarPlanner.setStartAndGoal(m_start, m_goal);
        m_thetaStarPlanner.generatePath(m_path, []{return false;});
        repaint();
    }
}

void CostmapWidget::mouseReleaseEvent(QMouseEvent *) {
    // qDebug() << m_path.size();
    // repaint();
}

void CostmapWidget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Backspace:
        m_gridMap->setData(std::vector<int8_t>(m_gridMap->data().size(), 0));
        m_costMap.updateCost();
        repaint();
        break;
    case Qt::Key_Delete:
        m_gridMap->setData(std::vector<int8_t>(m_gridMap->data().size(), 0));
        m_costMap.updateCost();
        repaint();
        break;
    case Qt::Key_Space:
        break;
    default:
        break;
    }
}
