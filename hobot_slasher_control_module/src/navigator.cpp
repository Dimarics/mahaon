#include "navigator.h"
#include <mrpt/config/CConfigFile.h>
#include <mrpt/nav/holonomic/CHolonomicND.h>

Navigator::Navigator(mrpt::nav::CRobot2NavInterface &robot_interface) : m_navigator(robot_interface)
{
    mrpt::config::CConfigFile config("config.ini");
    // m_navigator.changeRobotCircularShapeRadius(0.30);
    mrpt::math::CPolygon robotPolygon;
    robotPolygon.AddVertex(0.25,  0.25); // Право-верх
    robotPolygon.AddVertex(0.25, -0.25); // Право-низ
    robotPolygon.AddVertex(-0.25, -0.25); // Лево-низ
    robotPolygon.AddVertex(-0.25,  0.25); // Лево-верх
    m_navigator.changeRobotShape(robotPolygon);
    m_navigator.loadConfigFile(config);

    //=================== Initialize =========================
    m_navigator.initialize();
    std::cout << "✅ Навигатор инициализирован" << std::endl;
    // Выводим информацию о PTG
    size_t ptgCount = m_navigator.getPTG_count();
    std::cout << "📊 Создано PTG: " << ptgCount << std::endl;
    for (size_t i = 0; i < ptgCount; i++) {
        auto* ptg = m_navigator.getPTG(i);
        if (ptg) {
            std::cout << "   PTG[" << i << "]: " << ptg->getDescription() << std::endl;
        }
    }
    //========================================================
}

void Navigator::addWaypoint(double x, double y, double phi, bool relative) {
    // Для добавления waypoint используем navigate с новыми параметрами
    // Или можно использовать параметр waypoints
    mrpt::nav::CAbstractPTGBasedReactive::TNavigationParamsPTG params;
    params.target.target_coords = mrpt::math::TPose2D(x, y, phi);
    params.target.targetAllowedDistance = 0.30;
    params.target.target_frame_id = "map";
    params.target.targetIsRelative = relative;
    m_navigator.navigate(&params);
}

void Navigator::addWaypoints(const QList<mrpt::math::TPoint2D> &points) {
    mrpt::nav::TWaypointSequence seq;
    for (auto &point : points) {
        mrpt::nav::TWaypoint waypoint;
        waypoint.target = point;
        waypoint.allowed_distance = 0.3;
        // waypoint.target_heading = 0.0;
        // waypoint.allow_skip = false;
        seq.waypoints.push_back(waypoint);
    }
}

void Navigator::update() {
    try {
        // Выполняем шаг навигации
        m_navigator.navigationStep();

        // Проверяем состояние
        auto state = m_navigator.getCurrentState();

        switch (state) {
        case mrpt::nav::CAbstractNavigator::IDLE:
            // Навигация завершена или ещё не начата
            std::cout << "🎯 Цель достигнута!" << std::endl;
            break;
        case mrpt::nav::CAbstractNavigator::NAVIGATING:
            // Продолжаем движение
            std::cout << "NAVIGATING" << std::endl;
            break;
        case mrpt::nav::CAbstractNavigator::SUSPENDED:
            // Приостановлена
            std::cout << "SUSPENDED" << std::endl;
            break;
        case mrpt::nav::CAbstractNavigator::NAV_ERROR:
            // Ошибка
            auto error = m_navigator.getErrorReason();
            std::cerr << "❌ Ошибка навигации: " << error.error_msg << std::endl;
            break;
        }
    } catch (const std::exception& e) {
        std::cerr << "❌ Исключение в navigationStep(): " << e.what() << std::endl;
    }
}
