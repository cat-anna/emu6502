#include "emu/module/tty/tty_device.hpp"
#include "emu_core/bit_utils.hpp"
#include <fmt/format.h>

namespace emu::module::tty {

namespace {

constexpr BitField<> kCr0EnabledField{0, 1};
//  constexpr BitField<> kUCr0unused0{1,3};
constexpr BitField<> kCr0RateField{kBaudRageCr0BitOffset, 3};
//  constexpr BitField<> kUnused1{7,1};

struct ControlRegister0 {
    uint8_t enabled;
    uint8_t rate;

    static ControlRegister0 Deserialize(uint8_t reg_value) {
        ControlRegister0 r{0};
        r.enabled = kCr0EnabledField.Get(reg_value);
        r.rate = kCr0RateField.Get(reg_value);
        return r;
    }

    uint8_t Serialize() const {
        uint8_t r{0};
        kCr0EnabledField.Set(r, enabled);
        kCr0RateField.Set(r, rate);
        return r;
    }
};

} // namespace

uint64_t TtyDevice::BaudRateToByteRate(BaudRate br) {
    switch (br) {
    case BaudRate::b1200:
        return 1200 / 8;
    case BaudRate::b2400:
        return 2400 / 8;
    case BaudRate::b4800:
        return 4800 / 8;
    case BaudRate::b9600:
        return 9600 / 8;
    case BaudRate::b19200:
        return 19200 / 8;
    case BaudRate::b38400:
        return 38400 / 8;
    case BaudRate::b57600:
        return 57600 / 8;
    case BaudRate::b115200:
        return 115200 / 8;
    default:
        constexpr auto kCustomFlag =
            static_cast<std::underlying_type_t<BaudRate>>(BaudRate::bCustom);
        auto raw_value = static_cast<std::underlying_type_t<BaudRate>>(br);
        if ((kCustomFlag & raw_value) != 0) {
            return raw_value & (~kCustomFlag);
        }

        break;
    }
    throw std::runtime_error(fmt::format("TtyDevice: Invalid baudrate {:x}", br));
}

BaudRate TtyDevice::BaudRateFromInteger(int64_t v) {
    switch (v) {
    case 1200:
        return BaudRate::b1200;
    case 2400:
        return BaudRate::b2400;
    case 4800:
        return BaudRate::b4800;
    case 9600:
        return BaudRate::b9600;
    case 19200:
        return BaudRate::b19200;
    case 38400:
        return BaudRate::b38400;
    case 57600:
        return BaudRate::b57600;
    case 115200:
        return BaudRate::b115200;
    }
    throw std::runtime_error(fmt::format("TtyDevice: Invalid baudrate {:x}", v));
}

BaudRate TtyDevice::CustomBaudRate(uint64_t value) {
    return static_cast<BaudRate>(
        value | static_cast<std::underlying_type_t<BaudRate>>(BaudRate::bCustom));
}

TtyDevice::TtyDevice(std::istream *_input_stream,  //
                     std::ostream *_output_stream, //
                     Clock *_clock,                //
                     BaudRate _baudrate,           //
                     uint64_t _fifo_buffer_size,   //
                     bool _enabled)                //
    : input_stream(_input_stream),                 //
      output_stream(_output_stream),               //
      clock(_clock),                               //
      fifo_buffer_size(_fifo_buffer_size),         //
      enabled(!_enabled) {                         //
    if (fifo_buffer_size > 255) {
        throw std::runtime_error("TtyDevice: Fifo buffer size must fit in 8 bits");
    }
    SetEnabled(_enabled);
    SetRate(_baudrate);
}

void TtyDevice::SetEnabled(bool value) {
    if (value == enabled) {
        return;
    }
    enabled = value;
    if (enabled) {
        start_time = clock->Time();
        last_byte_time = 0;
    }
}

void TtyDevice::SetRate(BaudRate baud) {
    current_baudrate = baud;
    byte_rate_per_second = BaudRateToByteRate(baud);
}

std::optional<uint8_t> TtyDevice::DebugRead(Address_t address) const {
    switch (static_cast<Register>(address)) {
    case Register::kControl:
        return ControlRegister0{
            .enabled = static_cast<uint8_t>(enabled ? 1u : 0u),
            .rate = static_cast<uint8_t>(current_baudrate),
        }
            .Serialize();
    case Register::kInSize:
        return static_cast<uint8_t>(input_queue.size());
    case Register::kOutSize:
        return static_cast<uint8_t>(output_queue.size());
    case Register::kFifo:
        if (input_queue.empty()) {
            return static_cast<uint8_t>(0);
        } else {
            auto item = input_queue.front();
            input_queue.pop();
            return item;
        }
    }

    return std::nullopt;
}

uint8_t TtyDevice::Load(Address_t address) const {
    const_cast<TtyDevice *>(this)->UpdateBuffers();

    switch (static_cast<Register>(address)) {
    case Register::kControl:
        return ControlRegister0{
            .enabled = static_cast<uint8_t>(enabled ? 1u : 0u),
            .rate = static_cast<uint8_t>(current_baudrate),
        }
            .Serialize();
    case Register::kInSize:
        return static_cast<uint8_t>(input_queue.size());
    case Register::kOutSize:
        return static_cast<uint8_t>(output_queue.size());
    case Register::kFifo:
        if (input_queue.empty()) {
            return 0;
        } else {
            auto item = input_queue.front();
            input_queue.pop();
            return item;
        }
    }

    throw std::runtime_error(
        fmt::format("TtyDevice: Attempt to read address {:04x}", address));
}

void TtyDevice::Store(Address_t address, uint8_t value) {
    switch (static_cast<Register>(address)) {
    case Register::kControl: {
        UpdateBuffers(); //enabled might change so update buffers before
        auto cr0 = ControlRegister0::Deserialize(value);
        SetEnabled(cr0.enabled != 0);
        SetRate(static_cast<BaudRate>(cr0.rate));
        return;
    }
    case Register::kFifo:
        output_queue.push(value);
        if (output_queue.size() >= fifo_buffer_size) {
            output_queue.pop();
        }
        break;

    case Register::kInSize:
    case Register::kOutSize:
        //writting there does nothing
        break;
    }

    UpdateBuffers();

    if (address >= kDeviceMemorySize) {
        throw std::runtime_error(fmt::format(
            "TtyDevice: Attempt to write address {:04x} with {:02x}", address, value));
    }
}

uint64_t TtyDevice::ByteDelta() {
    auto time = (clock->Time() - start_time);
    auto total_bytes =
        static_cast<uint64_t>(time * static_cast<double>(byte_rate_per_second));
    auto delta = total_bytes - last_byte_time;
    last_byte_time = total_bytes;
    return delta;
}

void TtyDevice::UpdateBuffers() {
    auto delta = ByteDelta();
    if (delta == 0) {
        return;
    }

    if (enabled) {
        for (uint64_t i = 0; i < delta && !output_queue.empty(); ++i) {
            ++processed_output_bytes;
            auto byte = output_queue.front();
            output_queue.pop();
            if (output_stream != nullptr) {
                output_stream->write(reinterpret_cast<char *>(&byte), 1);
            }
        }
    }

    if (input_stream != nullptr) {
        for (uint64_t i = 0; i < delta && !input_stream->eof(); ++i) {
            uint8_t byte = 0;
            input_stream->read(reinterpret_cast<char *>(&byte), 1);
            if (!input_stream->eof()) {
                input_queue.push(byte);
                if (input_queue.size() >= fifo_buffer_size) {
                    input_queue.pop();
                }
                ++processed_input_bytes;
            }
        }
    }
}

} // namespace emu::module::tty
