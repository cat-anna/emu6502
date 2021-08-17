#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "emu_core/file_search.hpp"
#include "emu_core/memory_configuration_file.hpp"
#include <sstream>

namespace emu::test {
namespace {

using namespace ::testing;
using namespace std::string_view_literals;
using namespace std::string_literals;

class MemoryConfigFileTest : public testing::Test {
public:
    std::shared_ptr<emu::FileSearchMock> search_mock =
        std::make_shared<StrictMock<emu::FileSearchMock>>();
};

TEST_F(MemoryConfigFileTest, simple_test) {
    auto t = R"==(
memory:
- ram:
  offset: 0
  size: 0x0200
- rom:
  offset: 0x0200
  size: 0x0200
  image:
    file: test.bin
    offset: 0x01
- device:
  offset: 0xF000
  name: tty
  class: tty.simple
  config:
    a: b
    i: 5
    b: false
- include: test_config.yaml
)=="s;

    auto t2 = R"==(
memory:
- ram:
  offset: 0x1000
  size: 0x0100
)=="s;

    EXPECT_CALL(*search_mock, OpenFile("test_config.yaml", false))
        .WillOnce(Invoke([&](auto, auto) { //
            return std::make_shared<std::stringstream>(t2);
        }));
    EXPECT_CALL(*search_mock, PrependPath("test_config.yaml")).WillOnce(Invoke([&](auto) {
        return search_mock;
    }));

    try {
        auto config = LoadMemoryConfigurationFromString(t, search_mock.get());

        const MemoryConfigEntry i0{
            .name = "",
            .offset = 0,
            .entry_variant =
                MemoryConfigEntry::RamArea{
                    .image = std::nullopt,
                    .size = 0x0200,
                    .writable = true,
                },
        };
        const MemoryConfigEntry i1{
            .name = "",
            .offset = 0x0200,
            .entry_variant =
                MemoryConfigEntry::RamArea{
                    .image =
                        MemoryConfigEntry::RamArea::Image{
                            .file = "test.bin",
                            .offset = 0x01,
                        },
                    .size = 0x0200,
                    .writable = false,
                },
        };
        const MemoryConfigEntry i2{
            .name = "tty",
            .offset = 0xf000,
            .entry_variant =
                MemoryConfigEntry::MappedDevice{
                    .module_name = "tty",
                    .class_name = "simple",
                    .config = {{"a", "b"}, {"i", 5}, {"b", false}},
                },
        };
        const MemoryConfigEntry i3{
            .name = "",
            .offset = 0x1000,
            .entry_variant =
                MemoryConfigEntry::RamArea{
                    .image = std::nullopt,
                    .size = 0x0100,
                    .writable = true,
                },
        };
        auto expected = MemoryConfig{
            .entries = {i0, i1, i2, i3},
        };
        std::cout << StoreMemoryConfigurationToString(config);
        EXPECT_EQ(config, expected);
    } catch (const std::exception &e) {
        std::cout << "ERR: " << e.what() << "\n";
        throw;
    }
}

TEST_F(MemoryConfigFileTest, overrides) {
    auto t = R"==(
memory:
- rom:
  offset: 0x0200
  size: 0x0200
  image:
    file: $image_file
    offset: 0x01
- device:
  offset: 0xF000
  name: $dev_name
  class: $dev_class
  config:
    a: $dev_arg_text
    i: $dev_arg_int
    b: $dev_arg_bool
)=="s;

    const auto overrides = ConfigOverrides{
        {"image_file", "test.bin"},  //
        {"dev_name", "tty"},         //
        {"dev_class", "tty.simple"}, //
        {"dev_arg_text", "b"},       //
        {"dev_arg_bool", "false"},   //
        {"dev_arg_int", "5"},
    };

    try {
        auto config = LoadMemoryConfigurationFromString(t, search_mock.get(), overrides);

        const MemoryConfigEntry i1{
            .name = "",
            .offset = 0x0200,
            .entry_variant =
                MemoryConfigEntry::RamArea{
                    .image =
                        MemoryConfigEntry::RamArea::Image{
                            .file = "test.bin",
                            .offset = 0x01,
                        },
                    .size = 0x0200,
                    .writable = false,
                },
        };
        const MemoryConfigEntry i2{
            .name = "tty",
            .offset = 0xf000,
            .entry_variant =
                MemoryConfigEntry::MappedDevice{
                    .module_name = "tty",
                    .class_name = "simple",
                    .config = {{"a", "b"}, {"i", 5}, {"b", false}},
                },
        };

        auto expected = MemoryConfig{
            .entries = {i1, i2},
        };
        std::cout << StoreMemoryConfigurationToString(config);
        EXPECT_EQ(config, expected);
    } catch (const std::exception &e) {
        std::cout << "ERR: " << e.what() << "\n";
        throw;
    }
}

} // namespace
} // namespace emu::test
