#include "loader.hpp"

#include "../../common/errors.hpp"
#include "../../common/fileIO.hpp"
#include "../../common/tools.hpp"
#include "state.hpp"

void ExecutableLoader::load() {
    if (_state == nullptr) {
        throw RuntimeError("ExecutableLoader is not initialized");
    }

    _fileContents = readFileAsBytes(_state->config.target);
    if (_fileContents.size() < 4 || (!((_fileContents[0] == 0x7F) && (_fileContents[1] == 'E') &&
                                       (_fileContents[2] == 'L') && (_fileContents[3] == 'F')))) {
        throw InputError(std::string("Target file is not an ELF executable (first 4 bytes are: ") +
                         hexToString(_fileContents[0]) + ' ' + hexToString(_fileContents[1]) + ' ' +
                         hexToString(_fileContents[2]) + ' ' + hexToString(_fileContents[3]) + ")");
    }
    if (!(_fileContents[4] == 0x02)) {
        throw InputError("Target ELF executable is not for 64-bit architectures");
    }

    //
}

void ExecutableLoader::init(VMState* state) { _state = state; }
