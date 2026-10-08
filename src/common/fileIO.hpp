#pragma once

#include <string>

#include "bytebuff.hpp"

std::string readFileAsString(const std::string& filename);
ByteBuffer readFileAsBytes(const std::string& filename);

void writeToFile(const std::string& filename, const std::string& data);