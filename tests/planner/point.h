#ifndef POINT_H
#define POINT_H

#include <cmath>

template<typename T>
struct Point2D {
    Point2D() : x(0), y(0) {}
    Point2D(T _x, T _y) : x(_x), y(_y) {}
    template<typename Other>
    Point2D(const Point2D<Other> &other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}
    T x, y;

    Point2D operator-(const Point2D& o) const { return {x - o.x, y - o.y}; }
    Point2D operator+(const Point2D& o) const { return {x + o.x, y + o.y}; }
    Point2D operator*(T v) const { return {x * v, y * v}; }
    Point2D operator/(T v) const { return {x / v, y / v}; }

    T dot(const Point2D& o) const { return x * o.x + y * o.y; }
    T lengthSq() const { return x * x + y * y; }
    T distSq(const Point2D& p) const {
        T dx = x - p.x;
        T dy = y - p.y;
        return dx * dx + dy * dy;
    }
    inline T distance(const Point2D& p) const { return std::hypot(x - p.x, y - p.y); }
};

using Point = Point2D<int>;
using PointF = Point2D<double>;

#endif // POINT_H
