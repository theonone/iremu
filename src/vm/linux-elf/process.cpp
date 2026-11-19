#include "process.hpp"

#include "../x64/state.hpp"

Process::Process(VMState* state, int id, Process* parent) : _id(id), _parent(parent) {
    _vmem.init(state);
}

ExecutionContext Process::ctx() { return {.vmem = _vmem, .regs = _regs}; }

VirtualMemory& Process::vmem() { return _vmem; }
