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
    # ydlidar_launch_file = os.path.join(ydlidar_pkg_dir, 'launch', 'x3_ydlidar_launch.py')
    # ydlidar_launch = IncludeLaunchDescription(
    #     PythonLaunchDescriptionSource(ydlidar_launch_file),
    #     launch_arguments = {
    #         'params_file': ydlidar_params_file
    #     }.items()
    # )

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
    slam_toolbox_params_file = os.path.join(mahaon_pkg_dir, 'params', 'mapper_params.yaml')
    slam_toolbox_launch_file = os.path.join(slam_toolbox_pkg_dir, 'launch', 'online_sync_launch.py')
    launch_slam_toolbox = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(slam_toolbox_launch_file),
        launch_arguments = {
            'slam_params_file': slam_toolbox_params_file
        }.items()
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
        launch_slam_toolbox,
        nav2_bringup_launch,
        mahaon
    ])