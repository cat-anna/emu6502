#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "emu/module/random/mt19937_device.hpp"
#include "emu/module/random/random_device.hpp"
#include <sstream>

namespace emu::module::random::test {
namespace {

using namespace ::testing;
using namespace std::string_literals;

class Mt19937DeviceTest : public testing::Test {
public:
    Mt19937Device device{Mt19937Device::kDefaultSeed, &std::cout};

    using Register = Mt19937Device::Register;
};

TEST_F(Mt19937DeviceTest, ConsistentResult) {
    EXPECT_EQ(device.Load(static_cast<uint8_t>(Register::kSeed3)), 0xde);
    EXPECT_EQ(device.Load(static_cast<uint8_t>(Register::kSeed2)), 0xad);
    EXPECT_EQ(device.Load(static_cast<uint8_t>(Register::kSeed1)), 0xbe);
    EXPECT_EQ(device.Load(static_cast<uint8_t>(Register::kSeed0)), 0xef);

    EXPECT_EQ(device.Load(static_cast<uint8_t>(Register::kEntropy)), 0x39);
    EXPECT_EQ(device.Load(static_cast<uint8_t>(Register::kEntropy)), 0xe5);
    EXPECT_EQ(device.Load(static_cast<uint8_t>(Register::kEntropy)), 0xc5);
    EXPECT_EQ(device.Load(static_cast<uint8_t>(Register::kEntropy)), 0x6d);

    EXPECT_NO_THROW(device.Store(static_cast<uint8_t>(Register::kSeed0), 0xef));

    EXPECT_EQ(device.Load(static_cast<uint8_t>(Register::kEntropy)), 0x39);
    EXPECT_EQ(device.Load(static_cast<uint8_t>(Register::kEntropy)), 0xe5);
    EXPECT_EQ(device.Load(static_cast<uint8_t>(Register::kEntropy)), 0xc5);
    EXPECT_EQ(device.Load(static_cast<uint8_t>(Register::kEntropy)), 0x6d);
}

class RandomDeviceTest : public testing::Test {
public:
    RandomDevice device{&std::cout};
};

TEST_F(RandomDeviceTest, NoThrow) {
    EXPECT_NO_THROW((void)device.Load(0));
    EXPECT_NO_THROW((void)device.Load(0));
    EXPECT_NO_THROW((void)device.Load(0));
    EXPECT_NO_THROW((void)device.Load(0));
}

} // namespace
} // namespace emu::module::random::test
