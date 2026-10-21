#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow();

private:
    struct Point {
        Point(double _x, double _y) : x(_x), y(_y) {}
        double x, y;

        Point operator-(const Point& o) const { return {x - o.x, y - o.y}; }
        Point operator+(const Point& o) const { return {x + o.x, y + o.y}; }
        Point operator*(double s) const { return {x * s, y * s}; }

        double dot(const Point& o) const { return x * o.x + y * o.y; }
        double lengthSq() const { return x * x + y * y; }
        double distSq(const Point& b) const {
            double dx = x - b.x;
            double dy = y - b.y;
            return dx * dx + dy * dy;
        }
    };
    struct ExtremesResult {
        Point sameSide;
        Point otherSide;
    };

    const qreal R = 50;
    const qreal resolution = 10;

    QList<QPointF> m_obstacles;
    QPointF m_from;
    QPointF m_to;

    std::vector<size_t> obstaclesOnWay();
    bool lineIntersectsCircle(const Point& A, const Point& B, const Point& C, double R);
    std::vector<Point> findConnectedCircles(const std::vector<size_t>& obstacles_on_way);
    ExtremesResult extremes(const Point &A, const Point &B, const Point &circ);
    Point minFarthestPoint(const std::vector<Point> &obtacles);

    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // MAINWINDOW_H
