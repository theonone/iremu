#pragma once

#include <string>

std::string readFileAsString(const std::string& filename);

void writeToFile(const std::string& filename, const std::string& data);