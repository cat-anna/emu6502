#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "emu_core/memory_configuration_file.hpp"
#include <sstream>

namespace emu::test {
namespace {

using namespace ::testing;
using namespace std::string_view_literals;
using namespace std::string_literals;

class MemoryConfigFileTest : public testing::Test {
public:
};

TEST_F(MemoryConfigFileTest, test) {
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
  class: tty
  config:
    a: b
    i: 5
    b: false
)=="s;

    try {
        auto config = LoadMemoryConfigurationFromString(t);

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
                    .class_name = "tty",
                    .config = {{"a", "b"}, {"i", 5}, {"b", false}},
                },
        };
        auto expected = MemoryConfig{
            .entries = {i0, i1, i2},
        };
        EXPECT_EQ(config, expected);
    } catch (const std::exception &e) {
        std::cout << "ERR: " << e.what() << "\n";
        throw;
    }
}

} // namespace
} // namespace emu::test
