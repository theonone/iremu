#pragma once

#include "../../common/bytebuff.hpp"
#include "../x64/structs.hpp"
#include "../x64/vmem.hpp"

class ExecutableLoader {
   private:
    ByteBuffer _fileContents;

    void _validateELF();
    void _processPhEntries();

   public:
    ExecutableLoader() = default;
    void load(const std::string& filepath, VirtualMemory& vmem, Registers& regs);
};