#include "loader.hpp"

#include "../../common/errors.hpp"
#include "../../common/fileIO.hpp"
#include "../../common/tools.hpp"
#include "../x64/state.hpp"

void ExecutableLoader::_validateELF() {
    if (_fileContents.size() < 64) {
        throw InputError("Target executable is too short");
    }
    if ((!((_fileContents[0] == 0x7F) && (_fileContents[1] == 'E') && (_fileContents[2] == 'L') &&
           (_fileContents[3] == 'F')))) {
        throw InputError("Target file is not an ELF executable");
    }
    if (!(_fileContents[4] == 0x02)) {
        throw InputError("Target ELF executable is not for 64 bit architecures");
    }
    if (!(_fileContents[5] == 0x01)) {
        throw InputError("Target ELF executable uses big endian");
    }
    if (_fileContents[0x10] != 2) {
        throw InputError("Target is ELF, but not an executable " +
                         std::to_string(_fileContents[10]));
    }
    if (_fileContents[0x12] != 0x3e) {
        throw InputError("Target ELF executable is not for x86-64");
    }

    if (_fileContents[0x34] != 64) {
        throw InputError("Target ELF has an invalid ELF header length (not 64 bytes)");
    }
}

void ExecutableLoader::_processPhEntries() {
    uint64_t phoff = _fileContents[0x20];
    uint16_t phentsize = _fileContents[0x36];
    uint16_t phentnum = _fileContents[0x38];
    uint64_t shoff = _fileContents[0x28];
}

void ExecutableLoader::load() {
    if (_state == nullptr) {
        throw RuntimeError("ExecutableLoader is not initialized");
    }

    _fileContents = readFileAsBytes(_state->config.target);
    _validateELF();
    _processPhEntries();

    //
}

void ExecutableLoader::init(VMState* state) { _state = state; }
