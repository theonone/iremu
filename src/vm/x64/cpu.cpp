#include "cpu.hpp"

#include <chrono>
#include <iostream>
#include <thread>

#include "../../common/errors.hpp"
#include "vmState.hpp"

void CPU::run(ExecutionContext ctx, uint64_t n) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "running " << n << " CPU cycles on process " << ctx.pid << " thread " << ctx.tid
              << std::endl;
}

void CPU::init(VMState* vmState) { _vmState = vmState; }
