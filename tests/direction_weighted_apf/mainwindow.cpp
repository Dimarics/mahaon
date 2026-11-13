#include "mainwindow.h"
#include <QPainter>
#include <QMouseEvent>

MainWindow::MainWindow() : m_pos{NAN, NAN} {
    const qreal inscribed_r = _bounding_radius * _k_inscribed;
    const qreal inflation_r_px = _r_inflation * _scale;
    // const qreal inscribed_r_px = inscribed_r * _scale;

    m_apf = QImage(inflation_r_px * 2, inflation_r_px * 2, QImage::Format_ARGB32);
    m_apf.fill(Qt::transparent);
    for (int x = 0; x < m_apf.width(); ++x) {
        for (int y = 0; y < m_apf.height(); ++y) {
            const qreal mx = x / _scale;
            const qreal my = y / _scale;
            const qreal d = std::hypot(mx - _r_inflation, my - _r_inflation);
            uint8_t f;
            if (d > _r_inflation) {
                f = 0;
            } else if (d < inscribed_r) {
                f = 255;
            } else {
                f = static_cast<uint8_t>(255) * std::exp(-_cost_scaling_factor * (d - inscribed_r));
            }
            m_apf.setPixelColor(x, y, QColor(0, 0, 255, f));
        }
    }
    m_timer = new QTimer(this);
    m_timer->setInterval(30);
    m_timer->callOnTimeout(this, [this]() {
        const double speed = 0.15;
        const VecF u = (m_goal - m_pos).norm();
        double min_cost = 1.0;
        for (auto &obstacle : m_obstacles) {
            const VecF delta = (obstacle - m_pos) / _scale;
            const double dist_sq = delta.x * delta.x + delta.y * delta.y;
            if (dist_sq > _r_inflation * _r_inflation) continue;
            const double dist = std::sqrt(dist_sq);
            const VecF norm = delta / dist;
            const double c = norm.dot(u);
            if (c > 0 && dist < _r_inscribed) {
                min_cost = 0.0;
                break;
            }
            const double k_w = 0.8;
            const double w_dir = k_w + (1.0 - k_w) * (1.0 + c) / 2.0;

            double cost = 1.0 - std::clamp(std::exp(-_cost_scaling_factor * (dist - _r_inscribed)), 0.0, 1.0);
            // double _cost = c > 0 ? cost : 1.0 - w_dir * (1.0 - cost);
            double _cost = 1.0 - w_dir * (1.0 - cost);
            min_cost = std::min(_cost, min_cost);
        }
        m_pos = m_pos + u * speed * _scale * double(m_timer->interval()) / 1000.0 * min_cost;
        repaint();
    });
}

void MainWindow::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), QColor(33, 40, 48));

    // отрисовка препятствий
    painter.setRenderHint(QPainter::Antialiasing);
    // painter.setPen(QPen(Qt::blue, 0.0));
    // painter.setBrush(Qt::NoBrush);
    // for (auto &obstacle : m_obstacles) {
    //     const qreal r = _bounding_radius * _inflation_k * _scale;
    //     painter.drawEllipse(obstacle, r, r);
    // }
    for (auto &obstacle : m_obstacles) {
        VecF point = obstacle - VecF(1.0, 1.0) * double(m_apf.size().width()) / 2.0;
        painter.drawImage(QPointF(point.x, point.y), m_apf);
    }
    painter.setPen(Qt::red);
    painter.setBrush(Qt::NoBrush);
    const qreal inscribed_radius_px = _bounding_radius * _k_inscribed * _scale;
    for (auto &obstacle : m_obstacles) {
        painter.drawEllipse(QPointF(obstacle.x, obstacle.y), inscribed_radius_px, inscribed_radius_px);
    }
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::green);
    for (auto &obstacle : m_obstacles) {
        painter.drawEllipse(QPointF(obstacle.x, obstacle.y), 2, 2);
    }

    if (!std::isnan(m_pos.x)) {
        painter.setPen(QPen(Qt::gray, 0.0));
        painter.setBrush(Qt::NoBrush);
        const qreal bounding_radius_px = _bounding_radius * _scale;
        painter.drawEllipse(QPointF(m_pos.x, m_pos.y), bounding_radius_px, bounding_radius_px);
    }

    //--------------------------------------------------
    // for (auto &obstacle : m_obstacles) {
    //     const QPointF point = (obstacle - m_goal) / _scale;
    //     const qreal d_sq = point.x() * point.x() + point.y() * point.y();
    //     if (d_sq < _r_inflation * _r_inflation) {
    //         ranges << obstacle;
    //     }
    // }

    // painter.setPen(QPen(Qt::green, 0.0));
    // painter.setBrush(Qt::NoBrush);
    // for (auto &obstacle : ranges) {
    //     const qreal r = _bounding_radius * _k_inflation * _scale;
    //     painter.drawEllipse(obstacle, r, r);
    // }
    //--------------------------------------------------

    // нахождение крайних точек
    painter.setPen(QPen(Qt::green, 2.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(QPointF(m_pos.x, m_pos.y), QPointF(m_goal.x, m_goal.y));
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        m_pos = VecF(event->position().x(), event->position().y());
        m_goal = VecF(event->position().x(), event->position().y());
        m_timer->stop();
        repaint();
    }
    if (event->buttons() & Qt::RightButton) {
        VecF new_obstacle(event->pos().x(), event->pos().y());
        // if (m_obstacles.indexOf(new_obstacle) == -1) {
        m_obstacles.push_back(new_obstacle);
        // }
        repaint();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        m_goal = VecF(event->position().x(), event->position().y());
        repaint();
    }
    if (event->buttons() & Qt::RightButton) {
        VecF new_obstacle(event->pos().x(), event->pos().y());
        // if (m_obstacles.indexOf(new_obstacle) == -1) {
        m_obstacles.push_back(new_obstacle);
        // }
        repaint();
    }
    if (!m_timer->isActive()) m_timer->start();
}

void MainWindow::mouseReleaseEvent(QMouseEvent*) {
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Backspace:
        m_obstacles.resize(m_obstacles.size() - 1);
        repaint();
        break;
    case Qt::Key_Delete:
        m_obstacles.clear();
        repaint();
        break;
    case Qt::Key_Space:
        break;
    default:
        break;
    }
}
