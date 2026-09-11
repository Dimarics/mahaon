# установка ros
1. curl -fsSL https://rospian.github.io/rospian-repo/public/rospian-archive-keyring.asc | sudo gpg --dearmor | sudo tee /usr/share/keyrings/rospian-archive-keyring.gpg >/dev/null
2. echo "deb [arch=arm64 signed-by=/usr/share/keyrings/rospian-archive-keyring.gpg] https://rospian.github.io/rospian-repo trixie-jazzy main"   | sudo tee /etc/apt/sources.list.d/rospian.list
3. sudo apt install ros-jazzy-desktop или sudo apt install ros-jazzy-ros-base

настройка окружения:
4. echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc
5. source ~/.bashrc

проверка:
6. ros2 -h

<!-- # установка инструментов для сборки
1. sudo apt update && sudo apt install curl gnupg lsb-release
2. sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
3. echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu bookworm main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
4. sudo apt update
5. sudo apt install ros-dev-tools
6.
7. sudo apt remove python3-catkin-pkg -->

# ydlidar ros driver
git clone https://github.com/YDLIDAR/ydlidar_ros2_driver.git
colcon build --packages-select ydlidar_ros2_driver --cmake-args -DCMAKE_PREFIX_PATH=/home/mes/mola/install/YDLidar-SDK -DCMAKE_LIBRARY_PATH=/home/mes/mola/install/YDLidar-SDK/lib

ros2 run ydlidar_ros2_driver ydlidar_ros2_driver_node --ros-args --params-file ~/ros2_ws/src/ydlidar_ros2_driver/params/ydlidar_x3.yaml

запуск:
source ~/ros2_ws/install/setup.bash
ros2 launch ydlidar_ros2_driver x3_ydlidar_launch.py params_file:=/home/mes/ros2_ws/src/ydlidar_ros2_driver/params/ydlidar_x3.yaml

# tf
sudo apt install ros-jazzy-tf2 ros-jazzy-tf2-ros ros-jazzy-tf2-geometry-msgs

# geographic_info
<!-- git clone https://github.com/ros-geographic-info/geographic_info.git
cd geographic_info
git branch jazzy
cd .. -->
https://github.com/ros2-gbp/geographic_info-release/archive/refs/tags/rpm/ros-jazzy-geographic-msgs-1.0.6-2_9.zip
colcon build --packages-select geographic_msgs

# behavour_tree
sudo apt install libzmq3-dev
git clone https://github.com/BehaviorTree/BehaviorTree.CPP.git
cd behavour_tree
git branch 4.10.0
colcon build --packages-select behaviortree_cpp --cmake-args -DBTCPP_GROOT_INTERFACE=OFF -DCMAKE_PREFIX_PATH="/opt/ros/jazzy/include"
убрать проверки версии ament_index_cpp

# ros-navigation
sudo apt install ros-jazzy-backward-ros ros-jazzy-test-msgs ros-jazzy-angles libgraphicsmagick++1-dev libgraphicsmagick1-dev ros-jazzy-diagnostic-updater ros-jazzy-ament-cmake-google-benchmark
git clone https://github.com/ros-navigation/navigation2.git
cd navigation2
git branch jazzy
cd ../..
colcon build --packages-select nav2_ros_common --cmake-args DCMAKE_PREFIX_PATH=/opt/ros/jazzy/lib
colcon build --symlink-install
<!-- при ошибках в nav2_ros_common исправить в /test/CMakeList.txt:
заменить target_link_libraries на ament_target_dependencies и убрать <имя>:: -->
при ошибках в пакетах поменять в cmake <имя библиотеки>::<имя библиотеки> на ${<имя библиотеки>_TARGETS} ${_TARGETS}

# laser_scan_matcher
<!-- для ros1:
sudo apt-get install gsl-bin libgsl0-dev
git clone https://github.com/AndreaCensi/csm.git
cd csm
git branch 1.0.2
mkdir build
cmake -B build -DCMAKE_C_FLAGS="-D_GNU_SOURCE"
cmake --build build
sudo cmake --install build -->

для ros2:
sudo apt install libgsl-dev libjson-c-dev
git clone https://github.com/AlexKaravaev/csm.git
cd csm
git branch 1.0.2
mkdir build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/ros2_ws/install/csm"
cmake --build build
cmake --install build
cd ..
git clone https://github.com/AlexKaravaev/ros2_laser_scan_matcher.git
изменить в файле laser_scan_matcher.h в строке 43 #include <tf2_geometry_msgs/tf2_geometry_msgs.h> h на hpp
cd ..
colcon build --symlink-install

# собрать bond
1. git clone https://github.com/ros/bond_core.git
2. cd bond_core
3. git checkout jazzy
2. colcon build --packages-select smclib
3. colcon build --packages-select bond
4. colcon build --packages-select bondcpp
5. colcon build --packages-select slam_toolbox \
  --cmake-args \
  -Dbond_DIR=/home/mes/ros2_ws/install/bond/share/bond/cmake \
  -Dbondcpp_DIR=/home/mes/ros2_ws/install/bondcpp/share/bondcpp/cmake \
  -Dsmclib_DIR=/home/mes/ros2_ws/install/smclib/share/smclib/cmake

# установка SLAM Toolbox
<!-- 1. sudo apt install ros-jazzy-slam-toolbox -->
 1. sudo apt install libboost-all-dev libsuitesparse-dev libcereal-dev libceres-dev
 2. sudo apt install python3-empy python3-catkin-pkg python3-lark ros-jazzy-rviz-common ros-message-runtime ros-jazzy-rviz-default-plugins libtbb-dev
 3. mkdir ros2_ws
 4. cd ros2_ws
 5. mkdir src
 6. cd src
 7. git clone https://github.com/SteveMacenski/slam_toolbox.git
  . cd slam_toolbox
 8. git checkout jazzy
 9. cd ../../
10. rosdep init
11. rosdep update
12. rosdep install --from-paths src --ignore-src -r -y
13. colcon build --symlink-install

# запуск последовательности:
1. source ~/ros2_ws/install/setup.bash
2. ros2 launch ydlidar_ros2_driver x3_ydlidar_launch.py params_file:=/home/mes/ros2_ws/src/ydlidar_ros2_driver/params/ydlidar_x3.yaml
3. ros2 run tf2_ros static_transform_publisher 0 0 0 0 0 0 base_link laser
4. ros2 run ros2_laser_scan_matcher laser_scan_matcher --ros-args -p publish_odom:=/odom -p publish_tf:=true
5. ros2 launch slam_toolbox online_sync_launch.py

# команды:
colcon build --packages-select <имя пакета>
colcon build --symlink-install - собрать всё
ros2 topic list - список топиков (-t с типом -v подробно)
ros2 topic echo --once /имя_топика - прочитать топик разово
ros2 topic hz /имя_топика - частота
ros2 topic echo /odom | grep -E "position|orientation|x|y|z|w|twist"
ros2 topic echo /odom --once --field pose
ros2 topic echo /map --once > /home/mes/map_data.txt