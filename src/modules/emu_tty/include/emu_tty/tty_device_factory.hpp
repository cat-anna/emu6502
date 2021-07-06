#pragma once

#include "emu_core/device_factory.hpp"
#include "emu_core/stream_container.hpp"
#include "emu_tty/tty_device.hpp"
#include <cstdint>

namespace emu::tty {

struct DeviceInstance : public Device, std::enable_shared_from_this<DeviceInstance> {
    ~DeviceInstance() override = default;

    std::shared_ptr<Memory16> GetMemory() override { return device; }
    StreamContainer stream_container;
    std::shared_ptr<TtyDevice> device;
};

struct TtyDeviceFactory : public DeviceFactory {
    TtyDeviceFactory() = default;
    virtual ~TtyDeviceFactory() = default;

    std::shared_ptr<Device> CreateDevice(const std::string &name,
                                         const MemoryConfigEntry::MappedDevice &md,
                                         Clock *clock) override;
};

} // namespace emu::tty
