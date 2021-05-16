#include "base_test.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <optional>

namespace emu::cpu6502 {

using namespace emu::cpu6502::opcode;

class InstructionSetTest : public ::testing::Test {
public:
};

TEST_F(InstructionSetTest, VerifySupportedInstructions) {
    auto &implemented_instructions = Cpu6502::GetInstructionHandlerArray(InstructionSet::NMOS6502);
    auto instructions_map = Get6502InstructionSet();
    EXPECT_EQ(instructions_map.size(), 151);

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

} // namespace emu::cpu6502