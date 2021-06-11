#include "emu_6502/assembler/compiler.hpp"
#include "emu_6502/cpu/cpu.hpp"
#include <array>
#include <chrono>
#include <emu_core/base16.hpp>
#include <emu_core/build_config.hpp>
#include <emu_core/clock.hpp>
#include <emu_core/clock_steady.hpp>
#include <emu_core/memory.hpp>
#include <emu_core/memory_sparse.hpp>
#include <emu_core/program.hpp>
#include <gtest/gtest.h>

namespace emu::emu6502::test {

class ExecutionTest : public ::testing::Test {
public:
    using ClockType = std::conditional_t<kOptimizedBuild, ClockSteady, ClockSimple>;
    ClockType clock;
    MemorySparse16 memory{&clock, true, kDebugBuild};
    cpu::Cpu cpu{&clock, &memory, kDebugBuild, InstructionSet::NMOS6502Emu};

    std::vector<uint8_t> test_data;

    ExecutionTest() {}

    void SetUp() override {
        std::cout << "Using clock " << typeid(clock).name() << "\n";
        std::cout << "Frequency: " << clock.Frequency() << "\n";
        memory.Fill(kZeroPageBase, kMemoryPageSize);
        memory.Fill(kStackBase, kMemoryPageSize);
    }

    void PrepareRandomTestData(size_t len = 128) {
        test_data.resize(len);
        std::generate(test_data.begin(), test_data.end(), &ExecutionTest::RandomByte);
    }

    static uint8_t RandomByte() { return rand() & 0xFF; }

    auto RunCode(const std::string &code, std::chrono::milliseconds timeout = std::chrono::seconds{60}) {
        std::cout << "-----------CODE---------------------\n" << code << "\n";
        auto program = emu::emu6502::assembler::Compiler6502::CompileString(code, InstructionSet::NMOS6502Emu);
        std::cout << "-----------PROGRAM---------------------\n" << to_string(*program) << "\n";
        memory.WriteSparse(program->sparse_binary_code.sparse_map);
        std::cout << "-----------EXECUTION---------------------\n";
        auto start = std::chrono::steady_clock::now();
        try {
            clock.Reset();
            cpu.ExecuteFor(timeout);
            throw std::runtime_error("Exception was expected");
        } catch (const cpu::ExecutionHalted &) {
            std::cout << "-----------HALTED---------------------\n";
            // std::cout << e.what() << "\n";
        }
        auto end = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << fmt::format("Took {} microseconds\n", delta.count());

        std::cout << "-----------RESULT---------------------\n";
        std::cout << "Cycles: " << clock.CurrentCycle() << "\n";
        std::cout << "Lost cycles: " << clock.LostCycles() << "\n";
        return program;
    }
};

} // namespace emu::emu6502::test
