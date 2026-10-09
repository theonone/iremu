#include "tools.hpp"

#include <sstream>

std::string hexToString(int hex) {
    std::stringstream ss;
    ss << "0x" << std::hex << hex;
    return ss.str();
}