#pragma once

#include "../../common/bytebuff.hpp"

class VMState;

class ExecutableLoader {
   private:
    ByteBuffer _fileContents;
    VMState* _state = nullptr;

   public:
    ExecutableLoader() = default;
    void load();
    void init(VMState* state);
};