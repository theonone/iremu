#pragma once

#include "../../common/bytebuff.hpp"

class ExecutableLoader {
   private:
    ByteBuffer fileContents;

   public:
    ExecutableLoader() = default;
    void load(const std::string& target);
};