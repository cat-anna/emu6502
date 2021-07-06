#pragma once

#include "emu_core/memory.hpp"
#include <cstdint>
#include <emu_core/clock.hpp>
#include <iostream>
#include <list>
#include <queue>

namespace emu::tty {

constexpr Memory16::Address_t kDeviceMemorySize = 4;

enum class Register : Memory16::Address_t {
    kControl = 0,
    kInSize = 1,
    kOutSize = 2,
    kFifo = 3,
};

constexpr size_t kDefaultFifoBufferSize = 16;

enum class BaudRate : uint64_t {
    b1200,
    b2400,
    b4800,
    b9600,
    b19200,
    b38400,
    b57600,
    b115200,
    bCustom = 1llu << 63,
    bDefault = b9600,
};

constexpr uint8_t kBaudRageCr0BitOffset = 4;

class TtyDevice : public Memory16 {
public:
    TtyDevice(std::istream *_input_stream,                         //
              std::ostream *_output_stream,                        //
              Clock *_clock,                                       //
              BaudRate _baudrate = BaudRate::bDefault,             //
              uint64_t _fifo_buffer_size = kDefaultFifoBufferSize, //
              bool _enabled = false);

    uint8_t Load(Address_t address) const override;
    void Store(Address_t address, uint8_t value) override;

    [[nodiscard]] uint8_t Load(Register address) const {
        return Load(static_cast<Address_t>(address));
    }
    void Store(Register address, uint8_t value) {
        Store(static_cast<Address_t>(address), value);
    }

    [[nodiscard]] static uint64_t BaudRateToByteRate(BaudRate br);
    [[nodiscard]] static BaudRate CustomBaudRate(uint64_t value);
    [[nodiscard]] static BaudRate BaudRateFromInteger(int64_t v);

    void SetEnabled(bool value);
    void SetRate(BaudRate baud);

private:
    std::istream *const input_stream;
    std::ostream *const output_stream;
    Clock *const clock;
    uint64_t const fifo_buffer_size;

    BaudRate current_baudrate = BaudRate::bDefault;
    uint64_t byte_rate_per_second;
    uint64_t last_byte_time = 0;
    uint64_t processed_input_bytes = 0;
    uint64_t processed_output_bytes = 0;
    double start_time = 0;
    bool enabled = false;

    mutable std::queue<uint8_t> input_queue; //TODO: switch to something fancier
    std::queue<uint8_t> output_queue;        //TODO: switch to something fancier

    void UpdateBuffers();

    [[nodiscard]] uint64_t ByteDelta();
};

} // namespace emu::tty