#pragma once
#include <map>

#include "../common/config.hpp"

void fillConfig(VMConfig& conf, std::map<std::string, std::string>& args);

std::map<std::string, std::string> loadConfig(const std::string& conf);