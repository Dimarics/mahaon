#ifndef GRIDMAP_H
#define GRIDMAP_H

#include <cstdint>
#include <vector>
#include "point.h"

class GridMap
{
public:
    GridMap(uint32_t width, uint32_t height, double resolution = 1.0, double origin_x = 0.0, double origin_y = 0.0) :
        m_width(width), m_height(height), m_resolution(resolution), m_originX(origin_x), m_originY(origin_y),
        m_data(width * height, 0) {
    }

    inline uint32_t width() const { return m_width; }
    inline uint32_t height() const { return m_height; }
    inline double resolution() const { return m_resolution; }
    inline double originX() const { return m_originX; }
    inline double originY() const { return m_originY; }
    inline std::vector<int8_t> &data() { return m_data; }

    Point worldToMap(double wx, double wy) const {
        int x = int(wx - m_originX) / m_resolution;
        int y = int(wy - m_originY) / m_resolution;
        return {x, y};
    }
    Point worldToMap(const Point &p) const {
        return worldToMap(p.x, p.y);
    }
    PointF mapToWorld(int mx, int my) const {
        double x = (m_originX + (mx + 0.5) * m_resolution);
        double y = m_originY + (my + 0.5) * m_resolution;
        return {x, y};
    }
    PointF mapToWorld(double mx, double my) const {
        double x = (m_originX + (mx + 0.5) * m_resolution);
        double y = m_originY + (my + 0.5) * m_resolution;
        return {x, y};
    }
    PointF mapToWorld(const Point &p) const {
        return mapToWorld(p.x, p.y);
    }

    inline size_t getIndex(uint32_t x, uint32_t y) const { return x + y * m_width; }
    inline Point indexToCell(size_t index) const { return {int(index % m_width), int(index / m_width)}; }
    inline PointF indexToWorld(size_t index) const { return mapToWorld(indexToCell(index)); }

    inline void setData(const std::vector<int8_t> &data) { m_data = data; }
    void setCellInPos(double x, double y, int8_t value) {
        setCellInPos(Point(x, y), value);
    }
    void setCellInPos(const Point &point, int8_t value) {
        auto grid_pos = worldToMap(point);
        m_data[int(grid_pos.x) + int(grid_pos.y) * m_width] = value;
    }

private:
    uint32_t m_width;
    uint32_t m_height;
    double m_resolution;
    double m_originX;
    double m_originY;
    std::vector<int8_t> m_data;
};

#endif // GRIDMAP_H
