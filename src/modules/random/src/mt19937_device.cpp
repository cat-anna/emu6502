#include "emu/module/random/mt19937_device.hpp"
#include "emu_core/bit_utils.hpp"
#include <fmt/format.h>

namespace emu::module::random {

namespace {

size_t bit_offset(size_t v) {
    return v * 8;
}

} // namespace

Mt19937Device::Mt19937Device(IntType default_seed, std::ostream *verbose_output)
    : mt(default_seed), current_seed(default_seed), verbose_output(verbose_output) {
}

std::optional<uint8_t> Mt19937Device::DebugRead(Address_t address) const {
    switch (static_cast<Register>(address)) {
    case Register::kSeed0:
    case Register::kSeed1:
    case Register::kSeed2:
    case Register::kSeed3:
        return static_cast<uint8_t>((current_seed >> bit_offset(address)) & 0xFF);
    case Register::kEntropy:
        return std::nullopt;
    case Register::kCR0:
        return control_reg;
    }

    return std::nullopt;
}

uint8_t Mt19937Device::Load(Address_t address) const {
    switch (static_cast<Register>(address)) {
    case Register::kSeed0:
    case Register::kSeed1:
    case Register::kSeed2:
    case Register::kSeed3:
        return (current_seed >> bit_offset(address)) & 0xFF;
    case Register::kEntropy:
        return static_cast<uint8_t>(byte_distribution(mt));
    case Register::kCR0:
        return control_reg;
    }

    Error(fmt::format("Mt19937Device: Attempt to read address {:04x}", address));
}

void Mt19937Device::Store(Address_t address, uint8_t value) {
    switch (static_cast<Register>(address)) {
    case Register::kSeed0:
    case Register::kSeed1:
    case Register::kSeed2:
    case Register::kSeed3:
        current_seed = (current_seed & ~(0xFF << bit_offset(address))) |
                       (value << bit_offset(address));
        mt.seed(current_seed);
        return;
    case Register::kEntropy:
        if (verbose_output != nullptr) {
            (*verbose_output) << "Mt19937Device: Attempt to write to entropy register\n";
        }
        return;
    case Register::kCR0:
        control_reg = value;
        return;
    }

    Error(fmt::format("Mt19937Device: Attempt to write address {:04x} with {:02x}",
                      address, value));
}

} // namespace emu::module::random
