#pragma once

#include "../../common/bytebuff.hpp"
#include "../x64/structs.hpp"
#include "../x64/vmem.hpp"

class ExecutableLoader {
   private:
    ByteBuffer _fileContents;
    std::string _fp;
    VirtualMemory* _vmem = nullptr;
    Registers* _regs = nullptr;

    void _validateELF();
    void _processPhEntries();

   public:
    ExecutableLoader() = default;
    void load(const std::string& filepath, VirtualMemory* vmem, Registers* regs);
};