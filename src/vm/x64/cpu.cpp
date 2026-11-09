#include "cpu.hpp"

#include "../../common/errors.hpp"
#include "state.hpp"

void CPU::step() {}
void CPU::run() {
    if (_state == nullptr) {
        throw RuntimeError("Iremu bug: CPU object not initialized");
    }
}

const Registers& CPU::regs() const { return _regs; }

void CPU::init(VMState* state) { _state = state; }
