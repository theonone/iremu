#pragma once

#include "../../common/bytebuff.hpp"

class VMState;

class ExecutableLoader {
   private:
    ByteBuffer _fileContents;
    VMState* _state = nullptr;

    void _validateELF();
    void _processPhEntries();

   public:
    ExecutableLoader() = default;
    void load();
    void init(VMState* state);
};