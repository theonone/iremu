#pragma once

#include "../../common/bytebuff.hpp"
#include "../x64/structs.hpp"
#include "../x64/vmem.hpp"

class ExecutableLoader {
   private:
    ByteBuffer _fileContents;
    std::string _fp;
    ExecutionContext _ctx;
    uint64_t _heapBase;
    uint64_t _pageSize;
    bool _verbose;

    void _validateELF();
    void _processPhEntries();

    void _readField(void* var, uint64_t off, uint8_t bytes);

   public:
    ExecutableLoader(const std::string& filepath, ExecutionContext ctx);
};