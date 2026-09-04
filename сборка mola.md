# Сборка mola

cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_INSTALL_PREFIX=D:/Programs/YDLidar-SDK/build/install


# mrpt
нужны:
zstd
assimp
git clone https://github.com/MRPT/mrpt.git
cd mrpt
git checkout 2.15.19
git submodule update --init --recursive
mkdir build
cd build

В mrpt/libs/opengl/CMakeList.txt указать путь к libassimp.dll.a вручную

BUILD_mrpt-apps=ON
BUILD_APP_robot-map-gui=OFF
BUILD_mrpt-gui=OFF
MRPT_BUILD_TESTING=OFF
Eigen3_DIR="D:/Programs/eigen/install/share/eigen3/cmake"
MRPT_EIGEN_USE_EMBEDDED_VERSION=OFF

cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ '
    -DCMAKE_INSTALL_PREFIX="D:/Programs/mola/install/mrpt" '
    -DZSTD_INCLUDE_DIRS="D:/Programs/mola/install/zstd/include" '
    -DZSTD_LIBRARIES="D:/Programs/mola/install/zstd/lib/libzstd.a" '
    -DBUILD_SHARED_LIBS=ON '
    -DMRPT_BUILD_ASSIMP=ON '
    -DMRPT_BUILD_TESTING=OFF '
    -DEigen3_DIR="D:/Programs/eigen/install/share/eigen3/cmake" '
    -DMRPT_EIGEN_USE_EMBEDDED_VERSION=OFF

изменить в файле mola/install/mrtp/lib/cmake/mrtp-nanogui/mrtp-nanogui-config.cmake glfw на glfw3

# assimp
cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_INSTALL_PREFIX=D:/Programs/mola/install/assimp


# gtsam
BUILD_SHARED_LIBS=OFF
GTSAM_BUILD_TESTS=OFF
GTSAM_BUILD_EXAMPLES_ALWAYS=OFF
CMAKE_INSTALL_PREFIX="D:/Programs/mola/install/gtsam"

добавить:
файлы boost

на windows:
GTSAM_USE_SYSTEM_METIS=ON
GTSAM_SUPPORT_NESTED_DISSECTION=OFF


# boost
.\b2.exe toolset=gcc --with-chrono --with-date_time --with-filesystem --with-program_options --with-regex --with-serialization --with-thread --with-timer link=shared threading=multi variant=release stage --stagedir="D:/Programs/boost_1_91_0/build"

cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_INSTALL_PREFIX="D:/Programs/mola/install"

# zlib
1. git clone https://github.com/madler/zlib.git
2. cd zlib
3. git checkout v1.3.2
4. mkdir build
6. cd build
7. cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_INSTALL_PREFIX="D:/Programs/zlib/install"
8. mingw32-make
9. cmake --install .

# mp2p_icp
1. git clone https://github.com/MOLAorg/mp2p_icp.git
2. cd mp2p_icp
3. git checkout 2.12.0
4. git submodule update --init --recursive
5. mkdir build
6. cd build

cmake .. -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_INSTALL_PREFIX="D:/Programs/mola/install/mp2p_icp" -Dmola_common_DIR="D:/Programs/mola/install/mola_common/cmake" -DCMAKE_PREFIX_PATH="D:/Programs/mola/install/mrpt;D:/Programs/eigen/install" -Dglfw3_DIR="D:/Programs/glfw/install/lib/cmake/glfw3" -DEP_eigen3_DIR="D:/Programs/eigen/install/share/eigen3/make" -DMP2PICP_BUILD_APPLICATIONS=ON

# mola
cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_INSTALL_PREFIX="D:/Programs/mola/install" -DCMAKE_PREFIX_PATH="D:/Programs/mola/install"

cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_INSTALL_PREFIX="D:/Programs/mola/install"
mingw32-make.exe
cmake --install .

# glfw
cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_INSTALL_PREFIX="D:/Programs/glfw/install"

# eigen
1. git clone https://github.com/eigen-mirror/eigen.git
2. cd eigen
3. git checkout 5.0.0
4. mkdir build
5. cd build
6. cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_INSTALL_PREFIX="D:/Programs/eigen/install"
8. mingw32-make
7. cmake --install .

# wxWidgets
1. git clone https://github.com/wxWidgets/wxWidgets.git
2. cd wxWidgets
3. git checkout v3.3.3
4. git submodule update --init --recursive
5. mkdir cmake_build
6. cd cmake_build
7. cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_INSTALL_PREFIX="D:/Programs/mola/install/wxWidgets"
8. mingw32-make
9. cmake --install .


cmake .. -G "MinGW Makefiles" `
  -DCMAKE_CXX_COMPILER=g++ `
  -DCMAKE_INSTALL_PREFIX="D:/Programs/mola/install/mp2p_icp" `
  -Dmola_common_DIR="D:/Programs/mola/install/mola_common/cmake" `
  -DCMAKE_PREFIX_PATH="D:/Programs/mola/install/mrpt;D:/Programs/eigen/install" `
  -Dglfw3_DIR="D:/Programs/glfw/install/lib/cmake/glfw3" `
  -DEP_eigen3_DIR="D:/Programs/eigen/install/share/eigen3/make" `
  -DMP2PICP_BUILD_APPLICATIONS=ON `
  -DCMAKE_SHARED_LINKER_FLAGS="-L D:/Programs/mola/install/mrpt/lib -lmrpt-gui21519_GNU_x64 -lmrpt-nanogui21519_GNU_x64 -lmrpt-opengl21519_GNU_x64 -lmrpt-maps21519_GNU_x64 -lmrpt-obs21519_GNU_x64 -lmrpt-img21519_GNU_x64 -lmrpt-poses21519_GNU_x64 -lmrpt-math21519_GNU_x64 -lmrpt-system21519_GNU_x64 -lmrpt-core21519_GNU_x64 -lglfw3dll -lopengl32 -lwxmsw33ud_core_gcc_x64_custom -lwxbase33ud_gcc_x64_custom" `
  -DCMAKE_EXE_LINKER_FLAGS="-L D:/Programs/mola/install/mrpt/lib -lmrpt-gui21519_GNU_x64 -lmrpt-nanogui21519_GNU_x64 -lmrpt-opengl21519_GNU_x64 -lmrpt-maps21519_GNU_x64 -lmrpt-obs21519_GNU_x64 -lmrpt-img21519_GNU_x64 -lmrpt-poses21519_GNU_x64 -lmrpt-math21519_GNU_x64 -lmrpt-system21519_GNU_x64 -lmrpt-core21519_GNU_x64 -lglfw3dll -lopengl32 -lwxmsw33ud_core_gcc_x64_custom -lwxbase33ud_gcc_x64_custom"

  git config --global --add safe.directory D:/Programs/mola/mrpt