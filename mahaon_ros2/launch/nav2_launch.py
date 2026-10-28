import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
# from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.actions import LifecycleNode

def generate_launch_description():
    mahaon_pkg_dir = get_package_share_directory('mahaon_ros2')
    
    # запуск YDlidar
    ydlidar_pkg_dir = get_package_share_directory('ydlidar_ros2_driver')
    ydlidar_params_file = os.path.join(mahaon_pkg_dir, 'params', 'ydlidar_x3.yaml')
    ydlidar_launch = LifecycleNode(
        package='ydlidar_ros2_driver',
        executable='ydlidar_ros2_driver_node',
        name='ydlidar_ros2_driver_node',
        namespace='/',
        output='screen',
        emulate_tty=True,
        parameters=[ydlidar_params_file],
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
            'publish_tf': False,
            'publish_odom': '/odom'
        }]
    )

    # запуск nav2_bringup
    nav2_bringup_pkg_dir = get_package_share_directory('nav2_bringup')
    nav2_bringup_params_file = os.path.join(mahaon_pkg_dir, 'params', 'nav2_params_2.yaml')
    nav2_bringup_launch_file = os.path.join(nav2_bringup_pkg_dir, 'launch', 'navigation_launch.py')
    nav2_bringup_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(nav2_bringup_launch_file),
        launch_arguments={
            'params_file': nav2_bringup_params_file
        }.items()
    )

    # запуск mahaon
    mahaon = Node(
        package='mahaon_ros2',
        executable='mahaon',
        name='mahaon'
    )
    
    ###
    return LaunchDescription([
        ydlidar_launch,
        tf2_node,
        scan_matcher,
        nav2_bringup_launch,
        mahaon
    ])