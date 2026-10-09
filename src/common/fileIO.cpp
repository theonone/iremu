#include "fileIO.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

std::string readFileAsString(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to open file: " + filename);

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

ByteBuffer readFileAsBytes(const std::string& filename) {
    std::ifstream inputFile(filename, std::ios_base::binary);

    inputFile.seekg(0, std::ios_base::end);
    auto length = inputFile.tellg();
    inputFile.seekg(0, std::ios_base::beg);

    ByteBuffer buff;
    buff.resize(length);
    inputFile.read(reinterpret_cast<char*>(buff.data()), length);

    inputFile.close();
    return buff;
}

void writeToFile(const std::string& filename, const std::string& data) {
    if (filename.find("/") != std::string::npos) {
        std::filesystem::create_directories(std::filesystem::path(filename).parent_path());
    }

    std::ofstream file(filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file)
        throw std::runtime_error("Failed to open file for writing: " + filename);

    file.write(data.data(), static_cast<std::streamsize>(data.size()));
    if (!file)
        throw std::runtime_error("Failed to write file: " + filename);
}