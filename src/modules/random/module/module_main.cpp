#include "emu/module/random/device_factory.hpp"
#include "emu/module/random/symbol_factory.hpp"
#include "emu_core/plugins/plugin.hpp"

using namespace emu::module::random;

EMU_DEFINE_FACTORIES(Mt19937DeviceFactory, Mt19937DeviceSymbolFactory, default)
EMU_DEFINE_FACTORIES(Mt19937DeviceFactory, Mt19937DeviceSymbolFactory, mt19937)
EMU_DEFINE_FACTORIES(RandomDeviceFactory, RandomDeviceSymbolFactory, random)
