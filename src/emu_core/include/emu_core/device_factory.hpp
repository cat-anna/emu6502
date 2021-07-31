#pragma once

#include "clock.hpp"
#include "memory.hpp"
#include "memory_configuration_file.hpp"
#include <iostream>
#include <memory>
#include <string>

namespace emu {

struct Device {
    virtual ~Device() = default;
    virtual std::shared_ptr<Memory16> GetMemory() = 0;
    virtual size_t GetMemorySize() = 0;
};

struct DeviceFactory {
    virtual ~DeviceFactory() = default;

    virtual std::shared_ptr<Device>
    CreateDevice(const std::string &name, const MemoryConfigEntry::MappedDevice &md,
                 Clock *clock, std::ostream *verbose_output = nullptr) const = 0;
};

} // namespace emu
