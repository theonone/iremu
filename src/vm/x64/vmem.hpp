#pragma once

#include "../../common/bytebuff.hpp"

constexpr uint8_t MEM_PERM_NONE = 0;
constexpr uint8_t MEM_PERM_R = 1;
constexpr uint8_t MEM_PERM_W = 1 << 1;
constexpr uint8_t MEM_PERM_X = 1 << 2;
constexpr uint8_t MEM_PERM_RW = MEM_PERM_R | MEM_PERM_W;
constexpr uint8_t MEM_PERM_RX = MEM_PERM_R | MEM_PERM_X;
constexpr uint8_t MEM_PERM_RWX = MEM_PERM_R | MEM_PERM_W | MEM_PERM_X;
constexpr uint8_t MEM_PERM_WX = MEM_PERM_W | MEM_PERM_X;

class VirtualMemory {
   public:
   private:
};