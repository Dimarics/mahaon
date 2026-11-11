#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#pragma once

#include <yaml-cpp/yaml.h>

namespace app_config
{
    extern YAML::Node params;
    extern std::string package_dir;
    extern void setup();
};

#endif // APP_CONFIG_H
