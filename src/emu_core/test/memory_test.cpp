#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "emu_core/byte_utils.hpp"
#include "emu_core/clock.hpp"
#include "emu_core/memory/memory_block.hpp"
#include "emu_core/memory/memory_mapper.hpp"
#include "emu_core/program.hpp"
#include <sstream>

namespace emu::test {
namespace {

using namespace emu::memory;
using namespace ::testing;

using Address_t = MemoryBlock16::Address_t;

using namespace std::string_view_literals;
using namespace std::string_literals;

class MemoryTest : public testing::Test {
public:
    ClockSimple clock;

    MemoryMock16 mock_a;
    MemoryMock16 mock_b;
};

TEST_F(MemoryTest, MemoryBlock16) {
    MemoryBlock16::VectorType content{1_u8, 2_u8, 3_u8};
    content.resize(128);
    MemoryBlock16 mem{&clock, content, MemoryMode::kReadWrite, &std::cout, "ut"};

    EXPECT_EQ(mem.Load(0_addr), 1);
    EXPECT_EQ(mem.Load(2_addr), 3);
    EXPECT_EQ(mem.Load(64_addr), 0);
    EXPECT_EQ(mem.Load(120_addr), 0);

    EXPECT_NO_THROW(mem.Store(0_addr, 5));
    EXPECT_EQ(mem.Load(0_addr), 5);

    EXPECT_THROW(mem.Load(128_addr), std::runtime_error);
    EXPECT_THROW(mem.Load(200_addr), std::runtime_error);
    EXPECT_THROW(mem.Store(200_addr, 1_u8), std::runtime_error);
}

TEST_F(MemoryTest, MemoryMapper16) {
    MemoryMapper16 mapper{&clock, {}, true, &std::cout};

    mapper.MapArea({0x00_addr, 0x10_addr}, &mock_a);
    mapper.MapArea({0x20_addr, 0x30_addr}, &mock_b);

    Sequence seq;

    EXPECT_CALL(mock_a, Load(1_addr)).WillOnce(Return(1_u8));
    EXPECT_EQ(mapper.Load(0x01_addr), 1_u8);

    EXPECT_CALL(mock_b, Load(8_addr)).WillOnce(Return(40_u8));
    EXPECT_EQ(mapper.Load(40_addr), 40_u8);

    EXPECT_THROW(mapper.Load(20_addr), std::runtime_error);
    EXPECT_THROW(mapper.Load(65_addr), std::runtime_error);

    EXPECT_CALL(mock_a, Store(1_addr, 5_u8));
    EXPECT_NO_THROW(mapper.Store(1_addr, 5_u8));

    EXPECT_CALL(mock_b, Store(8_addr, 8_u8));
    EXPECT_NO_THROW(mapper.Store(40_addr, 8_u8));
}

} // namespace
} // namespace emu::test
