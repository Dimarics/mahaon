#ifndef POINT_H
#define POINT_H

#include <cmath>

template<typename T>
struct Point2D {
    Point2D() : x(0), y(0) {}
    Point2D(T _x, T _y) : x(_x), y(_y) {}
    // Point2D(const Point2D &other) : x(other.x), y(other.y) {}
    template<typename Other>
    Point2D(const Point2D<Other> &other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}
    T x, y;

    Point2D operator-(const Point2D& o) const { return {x - o.x, y - o.y}; }
    Point2D operator+(const Point2D& o) const { return {x + o.x, y + o.y}; }
    Point2D operator*(T v) const { return {x * v, y * v}; }
    Point2D operator/(T v) const { return {x / v, y / v}; }

    double dot(const Point2D& o) const { return x * o.x + y * o.y; }
    double lengthSq() const { return x * x + y * y; }
    double distSq(const Point2D& p) const {
        double dx = x - p.x;
        double dy = y - p.y;
        return dx * dx + dy * dy;
    }
    inline double distance(const Point2D& p) const { return std::hypot(x - p.x, y - p.y); }

    template<typename Type>
    Point2D<Type> toType() const { return {static_cast<Type>(x), static_cast<Type>(y)}; }
};

using Point = Point2D<int>;
using PointF = Point2D<double>;

#endif // POINT_H
