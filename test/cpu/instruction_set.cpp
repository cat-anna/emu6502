#include "base_test.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <optional>

using namespace emu::cpu::opcode;

class InstructionSet : public ::testing::Test {
public:
    using Cpu6502 = emu::cpu::Cpu6502;
};

TEST_F(InstructionSet, VerifySupportedInstructions) {
    auto implemented_instructions = Cpu6502::InitInstructionHandlerArray();
    auto missing_instructions = Get6502InstructionSet();
    std::cout << fmt::format("Expected instruction count: {}\n", missing_instructions.size());

    size_t current_count = 0;
    for (size_t index = 0; index < implemented_instructions.size(); ++index) {
        auto item = implemented_instructions[index];
        if (item != nullptr) {
            ++current_count;
            missing_instructions.erase(index);
        }
    }
    std::cout << fmt::format("Supported instruction count: {}\n", current_count);

    std::string text;
    for (auto [op, name] : missing_instructions) {
        if (!text.empty()) {
            text += " ";
        }
        text += name.mnemonic;
    }
    EXPECT_EQ(missing_instructions.size(), 0) << "MISSING: " << text;
}
