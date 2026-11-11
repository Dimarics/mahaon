#include "mahaon/app_config.h"
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <iostream>

namespace app_config
{
    YAML::Node params;
    std::string package_dir;
    void setup() {
        // определение пути к пакету ros2
        try {
            package_dir = ament_index_cpp::get_package_share_directory("mahaon_ros2");
        } catch(const std::exception& e) {
            std::cout << "[app_config] файл конфигурации не найден" << std::endl;
        }

        // загрузка файла конфигурации
        try {
            params = YAML::LoadFile(package_dir + "/params/config.yaml");
        } catch(const std::exception& e) {
            std::cout << "[app_config] файл конфигурации не найден" << std::endl;
        }
    }
};
