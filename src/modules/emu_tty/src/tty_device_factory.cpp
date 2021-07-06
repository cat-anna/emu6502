#pragma once

#include "emu_tty/tty_device_factory.hpp"
#include <cstdint>
#include <string>

using namespace std::string_literals;

namespace emu::tty {

namespace {} // namespace

std::shared_ptr<Device>
TtyDeviceFactory::CreateDevice(const std::string &name,
                               const MemoryConfigEntry::MappedDevice &md, Clock *clock) {

    auto instance = std::make_shared<DeviceInstance>();

    //

    instance->device = std::make_shared<TtyDevice>(
        // input_stream
        instance->stream_container.OpenBinaryInput(md.GetConfigItem("input", ""s)),
        // output_stream
        instance->stream_container.OpenBinaryOutput(md.GetConfigItem("input", ""s)),
        // clock
        clock,
        // baudrate
        TtyDevice::BaudRateFromInteger(md.GetConfigItem<int64_t>("baudrate", 9600)),
        // fifo_buffer_size
        md.GetConfigItem<int64_t>("buffer_size", kDefaultFifoBufferSize),
        // enabled
        md.GetConfigItem("enabled", false)
        //
    );

    return instance;
}

} // namespace emu::tty
