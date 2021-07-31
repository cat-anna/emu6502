#pragma once

#include "emu/module/random/mt19937_device.hpp"
#include "emu/module/random/random_device.hpp"
#include "emu_core/device_factory.hpp"
#include "emu_core/stream_container.hpp"
#include <cstdint>

namespace emu::module::random {

template <typename T>
struct DeviceInstance : public Device, std::enable_shared_from_this<DeviceInstance<T>> {
    ~DeviceInstance() override = default;

    std::shared_ptr<Memory16> GetMemory() override { return device; }
    size_t GetMemorySize() override { return T::kDeviceMemorySize; };
    StreamContainer stream_container;
    std::shared_ptr<T> device;
};

struct Mt19937DeviceFactory : public DeviceFactory {
    Mt19937DeviceFactory() = default;
    ~Mt19937DeviceFactory() override = default;

    std::shared_ptr<Device> CreateDevice(const std::string &name,
                                         const MemoryConfigEntry::MappedDevice &md,
                                         Clock *clock,
                                         std::ostream *verbose_output) const override {
        auto instance = std::make_shared<DeviceInstance<Mt19937Device>>();

        auto seed = md.GetConfigItem<int64_t>("seed", Mt19937Device::kDefaultSeed);

        instance->device = std::make_shared<Mt19937Device>(
            static_cast<Mt19937Device::IntType>(seed), verbose_output);
        return instance;
    }
};

struct RandomDeviceFactory : public DeviceFactory {
    RandomDeviceFactory() = default;
    ~RandomDeviceFactory() override = default;

    std::shared_ptr<Device> CreateDevice(const std::string &name,
                                         const MemoryConfigEntry::MappedDevice &md,
                                         Clock *clock,
                                         std::ostream *verbose_output) const override {
        auto instance = std::make_shared<DeviceInstance<RandomDevice>>();
        instance->device = std::make_shared<RandomDevice>(verbose_output);
        return instance;
    }
};

} // namespace emu::module::random
