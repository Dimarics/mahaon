#include "mainwindow.h"
#include <QPainter>
#include <QMouseEvent>
#include <queue>

bool MainWindow::lineIntersectsCircle(const Point& A, const Point& B, const Point& C, double R) {
    const double R2 = R * R;
    const Point CA = A - C;
    const Point CB = B - C;

    const double dA = CA.lengthSq();
    const double dB = CB.lengthSq();

    // Оба конца внутри окружности — пересечения нет
    if (dA < R2 && dB < R2) return false;

    // Хотя бы один конец внутри или на границе — пересечение есть
    if (dA <= R2 || dB <= R2) return true;

    // Оба конца снаружи: ищем ближайшую точку отрезка к центру
    const Point AB = B - A;
    const double abLenSq = AB.lengthSq();

    // Вырожденный отрезок (A == B): оба снаружи → нет пересечения
    if (abLenSq == 0.0) return false;

    // Параметр ближайшей точки на прямой, ограниченный отрезком [0, 1]
    double t = (C - A).dot(AB) / abLenSq;
    if (t < 0.0) t = 0.0;
    else if (t > 1.0) t = 1.0;

    const Point P = A + AB * t;      // ближайшая точка отрезка к центру
    const double dMin = (P - C).lengthSq();

    return dMin <= R2;
}

std::vector<MainWindow::Point> MainWindow::findConnectedCircles(const std::vector<size_t> &obstacles_on_way) {
    std::vector<Point> points;
    for (auto &obstacle : m_obstacles) {
        points.push_back(Point(obstacle.x(), obstacle.y()));
    }
    //---------------------------------------

    const size_t n = points.size();
    const double threshold = 4.0 * R * R;

    std::vector<bool> visited(n, false);
    std::vector<Point> result;
    std::queue<size_t> q;

    for (auto &index : obstacles_on_way) {
        visited[index] = true;
        q.push(index);
    }

    while (!q.empty()) {
        size_t cur = q.front();
        q.pop();
        result.push_back(points[cur]);

        for (size_t i = 0; i < n; ++i) {
            if (visited[i]) continue;
            if (points[cur].distSq(points[i]) <= threshold) {
                visited[i] = true;
                q.push(i);
            }
        }
    }
    return result;
}

MainWindow::MainWindow() {

}

std::vector<size_t> MainWindow::obstaclesOnWay() {
    std::vector<size_t> result;
    for (size_t i = 0; i < size_t(m_obstacles.size()); ++i) {
        if (lineIntersectsCircle(Point(m_from.x(), m_from.y()), Point(m_to.x(), m_to.y()),
                                    Point(m_obstacles.at(i).x(), m_obstacles.at(i).y()), R)) {
            result.push_back(i);
        }
    }
    return result;
}

MainWindow::Point MainWindow::minFarthestPoint(const std::vector<Point> &obstacles)
{
    // Коэффициенты прямой ax + by + c = 0
    double a = m_to.y() - m_from.y();
    double b = -(m_to.x() - m_from.x());
    double c = -(a * m_from.x() + b * m_from.y());

    double n = std::sqrt(a*a + b*b);
    if (n == 0.0) {
        return Point(0, 0); // m_from == m_to, вырожденный случай
    }

    double na = a / n, nb = b / n;

    double bestPlusDist  = -1.0; // максимум расстояния по положительной стороне
    double bestMinusDist = -1.0; // максимум расстояния по отрицательной стороне
    Point  bestPlus(0, 0);
    Point  bestMinus(0, 0);

    for (const auto &obs : obstacles) {
        // Две крайние точки этой окружности
        Point P1 = obs + Point{na * R, nb * R};
        Point P2 = obs - Point{na * R, nb * R};

        double f1 = a * P1.x + b * P1.y + c;
        double f2 = a * P2.x + b * P2.y + c;

        double d1 = std::abs(f1) / n;
        double d2 = std::abs(f2) / n;

        // Классифицируем по знаку f(P)
        if (f1 > 0) {
            if (d1 > bestPlusDist) { bestPlusDist = d1; bestPlus = P1; }
        } else if (f1 < 0) {
            if (d1 > bestMinusDist) { bestMinusDist = d1; bestMinus = P1; }
        }

        if (f2 > 0) {
            if (d2 > bestPlusDist) { bestPlusDist = d2; bestPlus = P2; }
        } else if (f2 < 0) {
            if (d2 > bestMinusDist) { bestMinusDist = d2; bestMinus = P2; }
        }
    }

    if (bestPlusDist < 0 && bestMinusDist < 0) {
        return Point(0, 0); // нет окружностей
    }
    if (bestPlusDist < 0)  return bestMinus; // все окружности с одной стороны
    if (bestMinusDist < 0) return bestPlus;

    return (bestPlusDist <= bestMinusDist) ? bestPlus : bestMinus;
}

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), QColor(33, 40, 48));

    // отрисовка препятствий
    painter.setRenderHint(QPainter::Antialiasing);
    for (auto &obstacle : m_obstacles) {
        painter.setPen(QPen(Qt::blue, 0.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(obstacle, R, R);
    }
    auto object = findConnectedCircles(obstaclesOnWay());
    for (auto &circle : object) {
        painter.setPen(QPen(Qt::red, 0.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPointF(circle.x, circle.y), R, R);
    }
    for (auto &obstacle : m_obstacles) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(127, 127, 127));
        painter.drawRect(QRectF(obstacle.x() - resolution / 2.0, obstacle.y() - resolution / 2.0, resolution, resolution));
    }

    // нахождение крайних точек
    if (!object.empty()) {
        Point ex = minFarthestPoint(object);
        painter.setPen(QPen(QColor(255, 127, 0), 2.0));
        painter.drawLine(m_from, QPointF(ex.x, ex.y));
        painter.drawLine(QPointF(ex.x, ex.y), m_to);
        painter.setPen(QPen(Qt::black, 0.0));
        painter.setBrush(QColor(255, 127, 0));
        painter.drawEllipse(QPointF(ex.x, ex.y), 3, 3);
    }
    //
    painter.setPen(QPen(Qt::green, 2.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(m_from, m_to);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_from = event->position();
        m_to = event->position();
        repaint();
    }
    if (event->buttons() & Qt::RightButton) {
        QPointF new_obstacle = (event->pos() / 10) * 10;
        if (m_obstacles.indexOf(new_obstacle) == -1) {
            m_obstacles << new_obstacle;
        }
        repaint();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_to = event->position();
        repaint();
    }
    if (event->buttons() & Qt::RightButton) {
        QPointF new_obstacle = (event->pos() / 10) * 10;
        if (m_obstacles.indexOf(new_obstacle) == -1) {
            m_obstacles << new_obstacle;
        }
        repaint();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
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
