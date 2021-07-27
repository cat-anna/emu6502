
#include "emu/module/tty/tty_device_factory.hpp"
#include <cstdint>
#include <string>

using namespace std::string_literals;

namespace emu::module::tty {

std::shared_ptr<Device>
TtyDeviceFactory::CreateDevice(const std::string &name,
                               const MemoryConfigEntry::MappedDevice &md, Clock *clock,
                               std::ostream *verbose_output) {

    auto instance = std::make_shared<TtyDeviceInstance>();

    std::istream *input_stream = nullptr;
    if (auto input = md.GetConfigItem("input", ""s); !input.empty()) {
        input_stream = instance->stream_container.OpenBinaryInput(input);
    }

    std::ostream *output_stream = nullptr;
    if (auto output = md.GetConfigItem("output", ""s); !output.empty()) {
        output_stream = instance->stream_container.OpenBinaryOutput(output);
    }

    auto baudrate =
        TtyDevice::BaudRateFromInteger(md.GetConfigItem<int64_t>("baudrate", 9600));
    auto fifo_buffer_size =
        md.GetConfigItem<int64_t>("buffer_size", kDefaultFifoBufferSize);
    auto enabled = md.GetConfigItem("enabled", false);

    instance->device = std::make_shared<TtyDevice>(input_stream, output_stream, clock,
                                                   baudrate, fifo_buffer_size, enabled);

    return instance;
}

} // namespace emu::module::tty
