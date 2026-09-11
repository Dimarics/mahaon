cmake_minimum_required(VERSION 3.5)
project(hobot_sl_ros2)

if(WIN32)
    # set(CMAKE_AUTOUIC ON)
    # set(CMAKE_AUTOMOC ON)
    # set(CMAKE_AUTORCC ON)

    # set(CMAKE_CXX_STANDARD 17)
    # set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
    # find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

    # target_include_directories(hobot_sl_ros2 PRIVATE)

    # target_link_libraries(hobot_sl_ros2 Qt${QT_VERSION_MAJOR}::Core)

    # include(GNUInstallDirs)
    # install(TARGETS hobot_sl_ros2
    #     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    #     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    # )
elseif(UNIX)
    find_package(ament_cmake REQUIRED)
    find_package(rclcpp REQUIRED)
    find_package(nav_msgs REQUIRED)
endif()

include_directories(
    include
    src
)

add_executable(hobot_sl
    src/main.cpp
    src/rosbridge.cpp
)

if(WIN32)
    include_directories(D:/Programs/ros2/include)
elseif(UNIX)
    ament_target_dependencies(hobot_sl
        rclcpp
        nav_msgs
    )

    install(TARGETS hobot_sl RUNTIME DESTINATION lib/${PROJECT_NAME})

    ament_package()
endif()
