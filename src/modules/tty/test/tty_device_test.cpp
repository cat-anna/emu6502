#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "emu/module/tty/tty_device.hpp"
#include "emu_core/clock.hpp"
#include <sstream>

namespace emu::module::tty::test {
namespace {

using namespace ::testing;
using namespace std::string_literals;

class TtyDeviceTest : public testing::Test {
public:
    static constexpr uint64_t kTestRate = 1;

    std::stringstream input;
    std::stringstream output;
    StrictMock<ClockMock> clock_mock;

    TtyDevice device{
        &input,
        &output,
        &clock_mock, //
        TtyDevice::CustomBaudRate(kTestRate),
        kDefaultFifoBufferSize,
        false,
    };

    uint64_t test_time = 0;

    void SetUp() override {
        EXPECT_CALL(clock_mock, Time()).WillRepeatedly(Invoke([this]() {
            return static_cast<double>(test_time);
        }));
        device.SetEnabled(true);
    }
};

TEST_F(TtyDeviceTest, OutOfBounds) {
    EXPECT_THROW(device.Load(kDeviceMemorySize), std::runtime_error);
    EXPECT_THROW(device.Store(kDeviceMemorySize, 0), std::runtime_error);
}

TEST_F(TtyDeviceTest, InitialState) {
    EXPECT_EQ(device.Load(Register::kControl), 0x11);
    EXPECT_EQ(device.Load(Register::kInSize), 0);
    EXPECT_EQ(device.Load(Register::kOutSize), 0);
    EXPECT_EQ(device.Load(Register::kFifo), 0);
}

TEST_F(TtyDeviceTest, ReadByte) {
    input << '\x0f';
    test_time = 1;
    EXPECT_EQ(device.Load(Register::kInSize), 1);
    EXPECT_EQ(device.Load(Register::kFifo), 0x0f);
    EXPECT_EQ(device.Load(Register::kInSize), 0);
}

TEST_F(TtyDeviceTest, WriteByte) {
    EXPECT_NO_THROW(device.Store(Register::kFifo, 0x0f));
    EXPECT_EQ(device.Load(Register::kOutSize), 1);
    test_time = 1;
    EXPECT_EQ(device.Load(Register::kOutSize), 0);

    EXPECT_EQ(output.str(), "\x0f"s);
}

TEST_F(TtyDeviceTest, ReadSome) {
    input << "0123456789";
    test_time = 5;
    EXPECT_EQ(device.Load(Register::kInSize), 5);
    EXPECT_EQ(device.Load(Register::kFifo), '0');
    EXPECT_EQ(device.Load(Register::kFifo), '1');
    EXPECT_EQ(device.Load(Register::kInSize), 3);
    test_time = 10;
    EXPECT_EQ(device.Load(Register::kInSize), 8);
    EXPECT_EQ(device.Load(Register::kFifo), '2');
    EXPECT_EQ(device.Load(Register::kInSize), 7);

    for (auto c : "345678"s) {
        EXPECT_EQ(device.Load(Register::kFifo), c);
    }

    EXPECT_EQ(device.Load(Register::kInSize), 1);
    EXPECT_EQ(device.Load(Register::kFifo), '9');
    EXPECT_EQ(device.Load(Register::kInSize), 0);
    EXPECT_EQ(device.Load(Register::kFifo), 0);
}

TEST_F(TtyDeviceTest, WriteSome) {
    EXPECT_NO_THROW(device.Store(Register::kFifo, '0'));
    EXPECT_EQ(device.Load(Register::kOutSize), 1);
    test_time = 1;
    EXPECT_EQ(device.Load(Register::kOutSize), 0);
    EXPECT_EQ(output.str(), "0"s);

    for (auto c : "123456789"s) {
        EXPECT_NO_THROW(device.Store(Register::kFifo, c));
    }

    EXPECT_EQ(device.Load(Register::kOutSize), 9);
    test_time = 10;
    EXPECT_EQ(device.Load(Register::kOutSize), 0);
    EXPECT_EQ(output.str(), "0123456789"s);
}

TEST_F(TtyDeviceTest, EnabledLaterRead) {
    input << "0123456789";
    test_time = 5;
    EXPECT_EQ(device.Load(Register::kInSize), 5);
    device.SetEnabled(false);
    test_time = 10;
    EXPECT_EQ(device.Load(Register::kInSize), 10);
    device.SetEnabled(true);
    test_time = 15;
    EXPECT_EQ(device.Load(Register::kInSize), 10);
}

TEST_F(TtyDeviceTest, EnabledLaterWrite) {
    for (auto c : "01234"s) {
        EXPECT_NO_THROW(device.Store(Register::kFifo, c));
    }
    EXPECT_EQ(device.Load(Register::kOutSize), 5);
    device.SetEnabled(false);
    test_time = 5;
    EXPECT_EQ(device.Load(Register::kOutSize), 5);
    device.SetEnabled(true);
    test_time = 6;
    EXPECT_EQ(device.Load(Register::kOutSize), 4);
}

} // namespace
} // namespace emu::module::tty::test
