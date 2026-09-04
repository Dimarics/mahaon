# необходимые пакеты
sudo apt install git
libeigen3-dev
libmrpt-dev
libgtsam-dev
libboost-timer-dev
libcppunit-dev
libcli11-dev
<!-- ament-lint -->

# настройки
cd ~
mkdir mola

# gtsam
1. mkdir install/gtsam
1. git clone https://github.com/borglab/gtsam.git
2. cd gtsam
4. git checkout 4.2.1
6. mkdir build
7. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/gtsam" -DBUILD_SHARED_LIBS=ON -DGTSAM_BUILD_TESTS=OFF -DGTSAM_BUILD_EXAMPLES_ALWAYS=OFF -DGTSAM_BUILD_UNSTABLE=ON -DGTSAM_USE_SYSTEM_EIGEN=ON

# mrpt
1. mkdir install/mrpt
2. git clone https://github.com/MRPT/mrpt.git
3. cd mrpt
4. git checkout 2.15.18
5. git submodule update --init --recursive
6. mkdir build
7. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mrpt" -DMRPT_BUILD_TESTING=OFF -DBUILD_mrpt-apps=ON -DMRPT_EIGEN_USE_EMBEDDED_VERSION=OFF -DENABLE_SSE2=OFF -DENABLE_SSE3=OFF -DENABLE_SSE4_1=OFF -DENABLE_SSE4_2=OFF -DENABLE_SSE4_A=OFF -DENABLE_NEON=ON -DMRPT_BUILD_ASSIMP=OFF -DBUILD_mrpt-containers=ON
7. cmake --build build
8. cmake --install build

# mola
1. git clone https://github.com/MOLAorg/mola.git
2. cd mola
3. git checkout 3.1.0
4. git submodule update --init --recursive

# mola_kernel
1. mkdir ../install/mola_kernel
2. cd mola_kernel
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_kernel" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt"

# mola_metric_maps
1. mkdir ../install/mola_metric_maps
2. cd mola_metric_maps
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_metric_maps" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt;/home/mes/mola/install/mp2p_icp"

# mola_pose_list
1. mkdir ../install/mola_pose_list
2. cd mola_pose_list
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_pose_list" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt"

# mola_relocalization
1. mkdir ../install/mola_relocalization
2. cd mola_relocalization
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_relocalization" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt;/home/mes/mola/install/mp2p_icp"

# mola_yaml
1. mkdir ../install/mola_yaml
2. cd mola_yaml
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_yaml" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt"

# mola_common
1. mkdir install/mola_common
2. git clone https://github.com/MOLAorg/mola_common.git
3. cd mola_common
4. git checkout 0.6.1
5. mkdir build
6. cmake -B build -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_common"

# mola_imu_preintegration
1. mkdir install/mola_imu_preintegration
2. git clone https://github.com/MOLAorg/mola_imu_preintegration.git
3. cd mola_imu_preintegration
4. git checkout 1.17.0
5. mkdir build
6. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_imu_preintegration" -DCMAKE_PREFIX_PATH="/home/mes/mola/install/mrpt"

# mp2p_icp
1. mkdir install/mp2p_icp
2. git clone https://github.com/MOLAorg/mp2p_icp.git
3. cd mp2p_icp
4. git checkout 2.11.0
5. git submodule update --init --recursive
6. mkdir build
7. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mp2p_icp" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt" -DMP2PICP_BUILD_TESTING=OFF

# mola_lidar_odometry
1. mkdir install/mola_lidar_odometry
2. git clone https://github.com/MOLAorg/mola_lidar_odometry.git
3. cd mola_lidar_odometry
4. git checkout 3.1.0
5. mkdir build
6. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_lidar_odometry" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt;/home/mes/mola/install/mp2p_icp"

# mola_state_estimation __all__
1. mkdir install/mola_state_estimation
2. git clone https://github.com/MOLAorg/mola_state_estimation.git
3. cd mola_state_estimation
4. git checkout 2.4.2

# mola_gtsam_factors
1. mkdir ../install/mola_gtsam_factors
2. cd mola_gtsam_factors
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_gtsam_factors" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt"

# mola_georeferencing
1. mkdir ../install/mola_georeferencing
2. cd mola_georeferencing
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_georeferencing" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt;/home/mes/mola/install/mp2p_icp" -DCMAKE_SHARED_LINKER_FLAGS="-L/home/mes/mola/install/gtsam"

<!-- # mola_state_estimation
1. mkdir ../install/mola_state_estimation
2. cd mola_state_estimation
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_state_estimation" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt" -->

# mola_state_estimation_simple
1. mkdir ../install/mola_state_estimation_simple
2. cd mola_state_estimation_simple
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_state_estimation_simple" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt"

# mola_state_estimation_smoother
1. mkdir ../install/mola_state_estimation_smoother
2. cd mola_state_estimation_smoother
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_state_estimation_smoother" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt;/home/mes/mola/install/gtsam"

# 
1. mkdir ../install/
2. cd 
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt"

# 
1. mkdir ../install/
2. cd 
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt"

# 
1. mkdir ../install/
2. cd 
3. mkdir build
4. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt"

# mola_sm_loop_closure
1. mkdir install/mola_sm_loop_closure
2. git clone https://github.com/MOLAorg/mola_sm_loop_closure.git
3. cd mola_sm_loop_closure
4. git checkout 1.2.2
5. git submodule update --init --recursive
5. mkdir build
6. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_sm_loop_closure" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt"


<!-- # mola_yaml
1. mkdir install/mola_yaml
2. git clone https://github.com/MOLAorg/mola_yaml.git
3. cd mola_yaml
4. git checkout 0.2.2
5. mkdir build
6. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_yaml" -DCMAKE_PREFIX_PATH="/home/mes/mola/install/mola_common;/home/mes/mola/install/mrpt" -->

<!-- # mola_kernel
1. mkdir install/mola_kernel
2. git clone https://github.com/MOLAorg/mola_kernel.git
3. cd mola_kernel
4. git checkout 0.2.2
5. mkdir build
6. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_kernel" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt" -DCMAKE_CXX_FLAGS="-I/home/mes/mola/install/mola_yaml/include" -DCMAKE_SHARED_LINKER_FLAGS="-L/home/mes/mola/install/mola_yaml/lib"

cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_kernel" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt" -->

# gtsam2mrpt_serial
1. mkdir install/gtsam2mrpt_serial
2. git clone https://github.com/MRPT/gtsam2mrpt_serial.git
3. cd gtsam2mrpt_serial
4. git checkout 0.2.0
5. mkdir build
6. cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/gtsam2mrpt_serial" -DCMAKE_PREFIX_PATH="/home/mes/mola/install/mrpt;/home/mes/mola/install/gtsam"




# mola_mapper_2d
1. mkdir install/mola_mapper_2d
2. git clone https://github.com/MOLAorg/mola_mapper_2d.git
3. cd mola_mapper_2d
4. mkdir build
5. добавить #include <mola_kernel/interfaces/NavStateFilter.h> в mola_mapper_2d/apps/mola-mapper-2d-cli.cpp
6. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/mola_mapper_2d" -DCMAKE_PREFIX_PATH="/home/mes/mola/install;/home/mes/mola/install/mrpt;/home/mes/mola/install/mp2p_icp"


# YDLidar-SDK
1. mkdir install/YDLidar-SDK
2. git clone https://github.com/YDLIDAR/YDLidar-SDK.git
3. cd YDLidar-SDK
4. git checkout V1.2.7
5. mkdir build
6. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/mola/install/YDLidar-SDK" -DBUILD_SHARED_LIBS=ON



# LaMa
1. mkdir LaMa
2. cd LaMa
3. mkdir install
4. git clone https://github.com/iris-ua/iris_lama.git
5. cd iris_lama
6. git checkout 1.3.1
7. mkdir build
8. cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/home/mes/LaMa/install"