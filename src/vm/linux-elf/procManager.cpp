#include "procManager.hpp"

#include <iostream>

#include "../../common/errors.hpp"
#include "../cfgProv.hpp"
#include "../x64/vmState.hpp"

ProcessManager::ProcessManager() {}

void assert(bool cond) {
    if (!cond)
        throw RuntimeError("test failed");
}

void ProcessManager::init(VMState* vmState) { _vmState = vmState; }

void ProcessManager::createProcess(int32_t pid, const std::string& elf, int32_t ppid) {
    _processes.emplace(std::piecewise_construct, std::make_tuple(pid),
                       std::make_tuple(_vmState, pid, ppid, elf));
}

// threads and processes will probably be alerting the scheduler about pausing/resuming, so we can
// just keep track whether there are ready ones or not
int ProcessManager::run() {
    if (_processes.size() == 0) {
        throw RuntimeError("ProcessManager: no processes to run");
    }
    uint64_t n = 999;
    auto it = _processes.begin();
    while (true) {
        if (it->second.state() == PROC_STATE_READY) {
            auto ctx = it->second.ctx();
            if (ctx.has_value()) {
                std::cout << "running process " << it->second.pid() << std::endl;
                // stack tests
                auto& stack = ctx->stack;
                uint64_t rsp = GLB_STACK_TOP;
                stack.push(12345);
                rsp -= 8;
                stack.updateRsp(rsp);

                assert(stack.pop() == 12345);
                std::cout << "test 1 passed" << std::endl;

                rsp += 8;
                stack.updateRsp(rsp);

                stack.push(222);
                rsp -= 8;
                stack.updateRsp(rsp);

                stack.push(333);
                rsp -= 8;
                stack.updateRsp(rsp);

                assert(stack.pop() == 333);
                std::cout << "test 2 passed" << std::endl;

                rsp += 8;
                stack.updateRsp(rsp);

                assert(stack.pop() == 222);
                std::cout << "test 3 passed" << std::endl;

                rsp += 8;
                stack.updateRsp(rsp);

                assert(stack.pop() == 111);
                std::cout << "test 4 passed" << std::endl;

                rsp += 8;
                stack.updateRsp(rsp);

                uint64_t addr = GLB_STACK_TOP - 256;

                stack.write8(addr, 0xAB);
                stack.write16(addr + 8, 0xABCD);
                stack.write32(addr + 16, 0xABCDEF12);
                stack.write64(addr + 24, 0xABCDEF1234567890);

                assert(stack.read8(addr) == 0xAB);
                std::cout << "test 5 passed" << std::endl;

                assert(stack.read16(addr + 8) == 0xABCD);
                std::cout << "test 6 passed" << std::endl;

                assert(stack.read32(addr + 16) == 0xABCDEF12);
                std::cout << "test 7 passed" << std::endl;

                assert(stack.read64(addr + 24) == 0xABCDEF1234567890);
                std::cout << "test 8 passed" << std::endl;

                uint64_t pageSize = ConfigProvider::get().page_size;

                rsp -= pageSize * 2 + 64;
                stack.updateRsp(rsp);

                // Access the bottom of the newly committed stack.
                addr = rsp - REDZONE;

                stack.write64(addr, 0xDEADBEEFCAFEBABE);
                assert(stack.read64(addr) == 0xDEADBEEFCAFEBABE);
                std::cout << "test 9 passed" << std::endl;

                for (uint64_t i = 0; i < pageSize * 3 / 8; ++i) {
                    stack.push(i + 1);
                    rsp -= 8;
                    stack.updateRsp(rsp);
                }

                // Verify the values are still there.
                for (uint64_t i = pageSize * 3 / 8; i > 0; --i) {
                    assert(stack.pop() == i);
                    rsp += 8;
                    stack.updateRsp(rsp);
                }
                std::cout << "tests 10 to whatever passed" << std::endl;

                stack.updateRsp(rsp);

                // The red zone boundary itself should be accessible.
                stack.write8(rsp - REDZONE, 0x42);

                // One byte beyond it should fail.
                bool caught = false;

                try {
                    stack.write8(rsp - REDZONE - 1, 0x42);
                } catch (const MemoryError&) {
                    caught = true;
                }

                assert(caught);
                std::cout << "test whatever+1 passed" << std::endl;

                // Access beyond the upper boundary should also fail.
                caught = false;

                try {
                    stack.write8(GLB_STACK_TOP, 0x42);
                } catch (const MemoryError&) {
                    caught = true;
                }

                assert(caught);
                std::cout << "test whatever+2 passed" << std::endl;

                //
                return 0;
                _vmState->cpu.run(ctx.value(), n);
            }
        }
        if (++it == _processes.end()) {
            it = _processes.begin();
        }
    }

    // Process& root = _processes[0];
    // return root.exitCode();
}
