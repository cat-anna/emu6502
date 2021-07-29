#include "emu/module/random/random_device.hpp"
#include "emu_core/bit_utils.hpp"
#include <fmt/format.h>

namespace emu::module::random {

RandomDevice::RandomDevice(std::ostream *verbose_output)
    : verbose_output(verbose_output) {
}

std::optional<uint8_t> RandomDevice::DebugRead(Address_t address) const {
    return std::nullopt;
}

uint8_t RandomDevice::Load(Address_t address) const {
    return static_cast<uint8_t>(byte_distribution(rd));
}

void RandomDevice::Store(Address_t address, uint8_t value) {
    if (verbose_output != nullptr) {
        (*verbose_output) << "RandomDevice: Attempt to write\n";
    }
}

} // namespace emu::module::random
