#pragma once

#include "emu/module/tty/tty_device.hpp"
#include "emu_core/device_factory.hpp"
#include "emu_core/stream_container.hpp"
#include <cstdint>

namespace emu::module::tty {

struct TtyDeviceInstance : public Device,
                           std::enable_shared_from_this<TtyDeviceInstance> {
    ~TtyDeviceInstance() override = default;

    std::shared_ptr<Memory16> GetMemory() override { return device; }
    size_t GetMemorySize() override { return kDeviceMemorySize; };
    StreamContainer stream_container;
    std::shared_ptr<TtyDevice> device;
};

struct TtyDeviceFactory : public DeviceFactory {
    TtyDeviceFactory() = default;
    virtual ~TtyDeviceFactory() = default;

    std::shared_ptr<Device> CreateDevice(const std::string &name,
                                         const MemoryConfigEntry::MappedDevice &md,
                                         Clock *clock,
                                         std::ostream *verbose_output = nullptr) override;
};

} // namespace emu::module::tty
