
#include "emu/module/tty/tty_symbol_factory.hpp"
#include "emu/module/tty/tty_device.hpp"
#include "emu_core/text_utils.hpp"
#include <cstdint>
#include <fmt/format.h>

namespace emu::module::tty {

SymbolDefVector
TtyDeviceSymbolFactory::GetSymbols(const MemoryConfigEntry &entry,
                                   const MemoryConfigEntry::MappedDevice &md) {

    auto base = entry.offset;
    auto upper_name = ToUpper(entry.name);

    SymbolDefVector r;

    auto emit_symbol = [&](auto name, auto value) {
        r.emplace_back(SymbolDefinition{
            .name = fmt::format("TTY_{}_{}", upper_name, name),
            .value = GetSymbolAddress(value),
            .segment = Segment::AbsoluteAddress,
        });
    };
    auto emit_alias = [&](auto name, auto value) {
        r.emplace_back(SymbolDefinition{
            .name = fmt::format("TTY_{}_{}", upper_name, name),
            .value = GetSymbolAddress(value),
            .segment = std::nullopt,
        });
    };

    emit_symbol("BASE_ADDRESS", base);
    emit_symbol("REGISTER_CONTROL", base + static_cast<uint8_t>(Register::kControl));
    emit_symbol("REGISTER_IN_COUNT", base + static_cast<uint8_t>(Register::kInSize));
    emit_symbol("REGISTER_OUT_COUNT", base + static_cast<uint8_t>(Register::kOutSize));
    emit_symbol("REGISTER_FIFO", base + static_cast<uint8_t>(Register::kFifo));

    emit_alias("CR0_ENABLED", static_cast<uint8_t>(1));

    emit_alias("CR0_BAUD_2400", static_cast<uint8_t>(BaudRate::b2400)
                                    << kBaudRageCr0BitOffset);
    emit_alias("CR0_BAUD_1200", static_cast<uint8_t>(BaudRate::b1200)
                                    << kBaudRageCr0BitOffset);
    emit_alias("CR0_BAUD_4800", static_cast<uint8_t>(BaudRate::b4800)
                                    << kBaudRageCr0BitOffset);
    emit_alias("CR0_BAUD_9600", static_cast<uint8_t>(BaudRate::b9600)
                                    << kBaudRageCr0BitOffset);
    emit_alias("CR0_BAUD_19200", static_cast<uint8_t>(BaudRate::b19200)
                                     << kBaudRageCr0BitOffset);
    emit_alias("CR0_BAUD_38400", static_cast<uint8_t>(BaudRate::b38400)
                                     << kBaudRageCr0BitOffset);
    emit_alias("CR0_BAUD_57600", static_cast<uint8_t>(BaudRate::b57600)
                                     << kBaudRageCr0BitOffset);
    emit_alias("CR0_BAUD_115200", static_cast<uint8_t>(BaudRate::b115200)
                                      << kBaudRageCr0BitOffset);

    return r;
}

} // namespace emu::module::tty