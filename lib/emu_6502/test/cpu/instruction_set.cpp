#include "base_test.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <optional>

namespace emu::emu6502::test {
namespace {

class InstructionSetTest : public ::testing::Test {
public:
    void CheckInstructionSet(InstructionSet is) {
        auto &implemented_instructions = Cpu::GetInstructionHandlerArray(is);
        auto instructions_map = GetInstructionSet(is);

        std::cout << fmt::format("Expected instruction count: {}\n", instructions_map.size());

        auto missing_instructions = instructions_map;
        size_t current_count = 0;
        for (size_t index = 0; index < implemented_instructions.size(); ++index) {
            auto item = implemented_instructions[index];
            if (item != nullptr) {
                ++current_count;
                missing_instructions.erase(static_cast<Opcode>(index));
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
};

TEST_F(InstructionSetTest, VerifySupportedInstructions) {
    auto instructions_map = GetInstructionSet(InstructionSet::NMOS6502);
    EXPECT_EQ(instructions_map.size(), 151);
    CheckInstructionSet(InstructionSet::NMOS6502);
}

TEST_F(InstructionSetTest, VerifySupportedInstructionsEmu) {
    auto instructions_map = GetInstructionSet(InstructionSet::NMOS6502Emu);
    EXPECT_EQ(instructions_map.size(), 152);
    CheckInstructionSet(InstructionSet::NMOS6502Emu);
}

} // namespace
} // namespace emu::emu6502::test