import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

def generate_launch_description():
    # запуск YDlidar
    ydlidar_pkg_dir = get_package_share_directory('ydlidar_ros2_driver')
    ydlidar_launch_file = os.path.join(
        ydlidar_pkg_dir,
        'launch',
        'x3_ydlidar_launch.py'
    )
    launch_ydlidar = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(ydlidar_launch_file)
    )

    # запуск static_transform_publisher
    tf2_node = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_tf_pub_laser',
        arguments=['0', '0', '0', '0', '0', '0', 'base_link', 'laser']
    )

    # запуск laser_scan_matcher
    scan_matcher = Node(
        package='ros2_laser_scan_matcher',
        executable='laser_scan_matcher',
        name='laser_scan_matcher',
        parameters=[{
            'use_imu': False,
            'use_odom': False,
            'base_frame': 'base_link',
            'fixed_frame': 'odom',
            'publish_tf': True,
            'publish_odom': '/odom'
        }]
    )

    # запуск SLAM toolbox
    slam_toolbox_pkg_dir = get_package_share_directory('slam_toolbox')
    slam_toolbox_launch_file = os.path.join(
        slam_toolbox_pkg_dir,
        'launch',
        'online_sync_launch.py'
    )
    launch_slam_toolbox = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(slam_toolbox_launch_file)
    )

    # запуск mahaon
    mahaon = Node(
        package='mahaon_ros2',
        executable='mahaon',
        name='mahaon'
    )
    
    ###
    return LaunchDescription([
        launch_ydlidar,
        tf2_node,
        scan_matcher,
        launch_slam_toolbox,
        mahaon
    ])