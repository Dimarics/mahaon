cmake_minimum_required(VERSION 3.5)
project(mahaon_ros2)

set(CMAKE_CXX_STANDARD 20)

if(UNIX)
    find_package(ament_cmake REQUIRED)
    find_package(nav_msgs REQUIRED)
    find_package(nav2_msgs REQUIRED)
    find_package(rclcpp REQUIRED)
    find_package(rclcpp_action REQUIRED)
    find_package(sensor_msgs REQUIRED)
    find_package(tf2 REQUIRED)
    find_package(tf2_geometry_msgs REQUIRED)
    find_package(tf2_ros REQUIRED)
    find_package(Boost REQUIRED CONFIG COMPONENTS system)
endif()

add_subdirectory(bno085)

include_directories(
    include
    src
)

add_executable(mahaon
    src/main.cpp
    src/controller.cpp
    # src/laser_scan_matcher.cpp
    src/imu.cpp
    src/stateestimator.cpp
    src/rosbridge.cpp
    src/serialport.cpp
    src/packetassembler.cpp
    src/webserversession.cpp
    src/webserver.cpp
    src/navigator.cpp
)

target_link_libraries(mahaon PRIVATE bno085)

if(WIN32)
    target_include_directories(mahaon PRIVATE
        D:/Programs/lib/ros2/include
        D:/Programs/lib/boost_1_91_0
        D:/Programs/lib/csm/sm
        D:/Programs/lib/navigation2
        D:/Programs/lib/fusioncore/fusioncore_core/include
    )
    target_link_libraries(mahaon PRIVATE ws2_32 mswsock)
elseif(UNIX)
    # include_directories(${Boost_INCLUDE_DIR})
    # target_link_libraries(hobot_sl PRIVATE ${Boost_INCLUDE_DIR})

    target_link_libraries(mahaon PRIVATE
        ${rclcpp_TARGETS}
        ${rclcpp_action_TARGETS}
        ${nav_msgs_TARGETS}
        ${sensor_msgs_TARGETS}
        ${tf2_TARGETS}
        ${tf2_ros_TARGETS}
        ${tf2_geometry_msgs_TARGETS}
        ${nav2_msgs_TARGETS}
    )

    install(TARGETS mahaon DESTINATION lib/${PROJECT_NAME})
    install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})
    install(DIRECTORY params DESTINATION share/${PROJECT_NAME})

    ament_package()
endif()
