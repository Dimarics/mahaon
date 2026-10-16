#include "navigator.h"
#include "mrpt/kinematics/CVehicleVelCmd_DiffDriven.h"
#include <mrpt/config/CConfigFile.h>
#include <mrpt/nav/holonomic/CHolonomicND.h>

Navigator::Navigator(mrpt::nav::CRobot2NavInterface &robot_interface) :
    m_navigator(robot_interface, true, false), m_robotInterface(robot_interface)
{
    // m_navigator.changeRobotCircularShapeRadius(0.30);
    // mrpt::math::CPolygon robotPolygon;
    // robotPolygon.AddVertex(0.15,  0.15); // Право-верх
    // robotPolygon.AddVertex( 0.15, -0.25); // Право-низ
    // robotPolygon.AddVertex(-0.15, -0.15); // Лево-низ
    // robotPolygon.AddVertex(-0.15,  0.15); // Лево-верх
    // m_navigator.changeRobotShape(robotPolygon);
    mrpt::config::CConfigFile config("reactive2d_config.ini");
    m_navigator.loadConfigFile(config);

    //=================== Initialize =========================
    // m_navigator.initialize();
    // std::cout << "Навигатор инициализирован" << std::endl;
    // Выводим информацию о PTG
    // size_t ptgCount = m_navigator.getPTG_count();
    // std::cout << "Создано PTG: " << ptgCount << std::endl;
    // for (size_t i = 0; i < ptgCount; i++) {
    //     auto* ptg = m_navigator.getPTG(i);
    //     if (ptg) {
    //         std::cout << "   PTG[" << i << "]: " << ptg->getDescription() << std::endl;
    //         // получить траектории
    //         for (size_t k = 0; k < ptg->getPathCount(); ++k) {
    //             std::cout << "path " << k << ": " << std::endl;
    //             for (size_t step = 0; step < ptg->getPathStepCount(k); ++step) {
    //                 mrpt::math::TPose2D pose = ptg->getPathPose(k, step);
    //                 std::cout << "x: " << pose.x << " y: " << pose.y << " yaw: " << mrpt::RAD2DEG(pose.phi) << std::endl;
    //             }
    //         }
    //     }
    // }
    //========================================================
}

void Navigator::addWaypoint(double x, double y, double phi, bool relative) {
    // m_targetX = x;
    // m_targetY = y;

    // Для добавления waypoint используем navigate с новыми параметрами
    // Или можно использовать параметр waypoints
    m_navigationParamsPTG.target.target_coords = mrpt::math::TPose2D(x, y, phi);
    m_navigationParamsPTG.target.targetAllowedDistance = 0.30;
    // m_navigationParamsPTG.target.target_frame_id = "map";
    m_navigationParamsPTG.target.targetIsRelative = relative;
    m_navigator.navigate(&m_navigationParamsPTG);
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
    // moveToPoint();

    try {
        // Выполняем шаг навигации
        m_navigator.navigationStep();

        // Проверяем состояние
        auto state = m_navigator.getCurrentState();

        switch (state) {
        case mrpt::nav::CAbstractNavigator::IDLE:
            // Навигация завершена или ещё не начата
            std::cout << "Цель достигнута!" << std::endl;
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
            std::cerr << "Ошибка навигации: " << error.error_msg << std::endl;
            break;
        }
    } catch (const std::exception& e) {
        std::cerr << "Исключение в navigationStep(): " << e.what() << std::endl;
    }
}

void Navigator::moveToPoint() {
    mrpt::math::TPose2D current_pos;
    mrpt::math::TTwist2D current_vel_global;
    mrpt::system::TTimeStamp timestamp;
    mrpt::math::TPose2D current_odometry;
    std::string frame_id;
    m_robotInterface.getCurrentPoseAndSpeeds(current_pos, current_vel_global, timestamp, current_odometry, frame_id);

    double dx = m_targetX - current_pos.x;
    double dy = m_targetY - current_pos.y;
    double distance = std::hypot(dx, dy);
    // std::cout << "=====================================" << std::endl;
    // std::cout << "target distance: " << distance
    //           << "; target x: " << m_targetX
    //           << "; target y: " << m_targetY << std::endl;

    if (distance < m_arrRadius) {
        m_robotInterface.stop();
        // std::cout << "goal reached" << std::endl;
        return;
    }

    double target_angle = std::atan2(dy, dx);
    double angle_error = mrpt::math::angDistance(current_pos.phi, target_angle);
    // std::cout << "target angle: " << mrpt::RAD2DEG(target_angle)
    //           << "; current angle:" << mrpt::RAD2DEG(current_pos.phi)
    //           << "; angle error: " << mrpt::RAD2DEG(angle_error) << std::endl;

    double lin_vel = std::min(m_maxLinear, distance);
    double ang_vel = std::clamp(m_kpAngle * angle_error, -m_maxAngular, m_maxAngular);

    if (std::abs(angle_error) > mrpt::DEG2RAD(20)) {
        lin_vel = 0.0;
    }

    auto cmd = mrpt::kinematics::CVehicleVelCmd_DiffDriven();
    cmd.lin_vel = lin_vel;
    cmd.ang_vel = ang_vel;
    m_robotInterface.changeSpeeds(cmd);
}
