#include "cpu.hpp"

#include <chrono>
#include <iostream>
#include <thread>

#include "../../common/errors.hpp"
#include "vmState.hpp"

// remember to call updateRsp in stack at the end of a cycle
CPUInterrupt CPU::run(ExecutionContext ctx, uint64_t n) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "running " << n << " CPU cycles on process " << ctx.pid << " thread " << ctx.tid
              << std::endl;
    return {.reason = CPU_INT_CYCLES, .remaining = 0};
}

void CPU::init(VMState* vmState) { _vmState = vmState; }
