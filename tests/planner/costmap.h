#ifndef COSTMAP_H
#define COSTMAP_H

#include "gridmap.h"
#include <memory>

class CostMap {
public:
    enum Cost: uint8_t {
        FREE_SPACE = 0,
        INSCRIBED_INFLATED_OBSTACLE = 253,
        LETHAL_OBSTACLE = 254,
        NO_INFORMATION = 255
    };
    CostMap();
    inline void setGridMap(const std::shared_ptr<GridMap> &grid_map) {
        m_gridMap = grid_map;
        updateCost();
    }
    void updateCost();
    uint8_t &getCost(uint32_t x, uint32_t y) { return m_data[m_gridMap->getIndex(x, y)]; }
    void setCost(uint32_t x, uint32_t y, uint8_t cost) { m_data[m_gridMap->getIndex(x, y)] = cost; }
    std::vector<uint8_t> &cost() { return m_data; }

    inline uint32_t width() { return m_gridMap->width(); }
    inline uint32_t height() { return m_gridMap->height(); }

    inline Point worldToMap(double wx, double wy) const {
        return m_gridMap->worldToMap(wx, wy);
    }
    inline Point worldToMap(const Point &p) const {
        return m_gridMap->worldToMap(p);
    }
    inline PointF mapToWorld(int mx, int my) const {
        return m_gridMap->mapToWorld(mx, my);
    }
    inline PointF mapToWorld(const Point &p) const {
        return m_gridMap->mapToWorld(p);
    }

private:
    double m_inscribedRadius;
    double m_inflationRadius;
    double m_costScalingFactor;
    std::shared_ptr<GridMap> m_gridMap;
    std::vector<uint8_t> m_data;
};

#endif // COSTMAP_H
