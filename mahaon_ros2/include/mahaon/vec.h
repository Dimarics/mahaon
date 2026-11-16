#ifndef VEC_H
#define VEC_H

#include <cmath>

template<typename T>
struct Vec2D {
    Vec2D() : x(0), y(0) {}
    Vec2D(T _x, T _y) : x(_x), y(_y) {}
    // Point2D(const Point2D &other) : x(other.x), y(other.y) {}
    template<typename Other>
    Vec2D(const Vec2D<Other> &other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}
    T x, y;

    Vec2D operator-(const Vec2D& o) const { return {x - o.x, y - o.y}; }
    Vec2D operator+(const Vec2D& o) const { return {x + o.x, y + o.y}; }
    Vec2D operator*(T v) const { return {x * v, y * v}; }
    Vec2D operator/(T v) const { return {x / v, y / v}; }

    double dot(const Vec2D& o) const { return x * o.x + y * o.y; }
    double lengthSq() const { return x * x + y * y; }
    double distSq(const Vec2D& p) const {
        double dx = x - p.x;
        double dy = y - p.y;
        return dx * dx + dy * dy;
    }
    inline double distance(const Vec2D& p) const { return std::hypot(x - p.x, y - p.y); }
    inline Vec2D norm() const { return *this / std::hypot(x, y); }

    template<typename Type>
    Vec2D<Type> toType() const { return {static_cast<Type>(x), static_cast<Type>(y)}; }
};

using Vec = Vec2D<int>;
using VecF = Vec2D<double>;

#endif // VEC_H
