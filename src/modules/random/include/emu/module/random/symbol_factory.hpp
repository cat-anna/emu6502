#pragma once

#include "emu/module/random/mt19937_device.hpp"
#include "emu_core/symbol_factory.hpp"
#include <cstdint>
#include <memory>

using namespace std::string_literals;

namespace emu::module::random {

struct Mt19937DeviceSymbolFactory : public SymbolFactory {
    Mt19937DeviceSymbolFactory() = default;
    ~Mt19937DeviceSymbolFactory() override = default;

    static constexpr auto kClassName = "MT19937";

    SymbolDefVector GetSymbols(const MemoryConfigEntry &entry,
                               const MemoryConfigEntry::MappedDevice &md) const override {
        auto base = entry.offset;
        SymbolDefVectorBuilder r{kClassName, entry.name};
        using Reg = Mt19937Device::Register;
        r.EmitSymbol("BASE_ADDRESS"s, base);
        r.EmitSymbol("REGISTER_SEED_0"s, base, Reg::kSeed0);
        r.EmitSymbol("REGISTER_SEED_1"s, base, Reg::kSeed1);
        r.EmitSymbol("REGISTER_SEED_2"s, base, Reg::kSeed2);
        r.EmitSymbol("REGISTER_SEED_3"s, base, Reg::kSeed3);
        r.EmitSymbol("REGISTER_ENTROPY"s, base, Reg::kEntropy);
        r.EmitSymbol("REGISTER_CONTROL_0"s, base, Reg::kCR0);
        return r.entries;
    }
};

struct RandomDeviceSymbolFactory : public SymbolFactory {
    RandomDeviceSymbolFactory() = default;
    ~RandomDeviceSymbolFactory() override = default;

    static constexpr auto kClassName = "RANDOM";

    SymbolDefVector GetSymbols(const MemoryConfigEntry &entry,
                               const MemoryConfigEntry::MappedDevice &md) const override {
        auto base = entry.offset;
        SymbolDefVectorBuilder r{kClassName, entry.name};
        r.EmitSymbol("BASE_ADDRESS"s, base);
        r.EmitSymbol("REGISTER_ENTROPY"s, base);
        return r.entries;
    }
};

} // namespace emu::module::random
