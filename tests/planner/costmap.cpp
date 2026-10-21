#include "costmap.h"
#include "qdebug.h"
#include <QElapsedTimer>

CostMap::CostMap()
    : m_inscribedRadius(0.15)
    , m_inflationRadius(0.5)
    , m_costScalingFactor(8.0)
    , m_gridMap(nullptr) { }

void CostMap::updateCost() {
    if (!m_gridMap) return;
    m_data.resize(m_gridMap->data().size());
    std::vector<PointF> obstacles;
    for (size_t i = 0; i < m_data.size(); ++i) {
        switch (m_gridMap->data()[i]) {
        case -1:
            m_data[i] = NO_INFORMATION;
            break;
        case 100:
            m_data[i] = LETHAL_OBSTACLE;
            obstacles.push_back(m_gridMap->indexToWorld(i));
            break;
        default:
            m_data[i] = FREE_SPACE;
            break;
        }
    }
    // QElapsedTimer t;
    // t.start();
    for (size_t i = 0; i < m_data.size(); ++i) {
        switch (m_data[i]) {
        case NO_INFORMATION:
        case FREE_SPACE: {
            double d = INFINITY;
            auto cell_pos = m_gridMap->indexToWorld(i);
            for (auto &obstacle : obstacles) {
                d = std::min(d, cell_pos.distSq(obstacle));
            }
            d = sqrt(d);
            d *= 0.005;
            if (d <= m_inscribedRadius) {
                m_data[i] = INSCRIBED_INFLATED_OBSTACLE;
            } else if (d > m_inflationRadius) {
                m_data[i] = FREE_SPACE;
            } else {
                m_data[i] = static_cast<uint8_t>((INSCRIBED_INFLATED_OBSTACLE - 1) *
                                                 std::exp(-m_costScalingFactor * (d - m_inscribedRadius)));
            }
            break;
        }
        default:
            break;
        }
    }
    // qDebug() << m_data.size() << t.elapsed();
}
