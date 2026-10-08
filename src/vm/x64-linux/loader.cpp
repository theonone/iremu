#include "loader.hpp"

#include "../../common/fileIO.hpp"

void ExecutableLoader::load(const std::string& target) {
    fileContents = readFileAsBytes(target);
    //
}
