#include "emu/module/tty/tty_device_factory.hpp"
#include "emu/module/tty/tty_symbol_factory.hpp"
#include "emu_core/plugins/plugin.hpp"

using namespace emu::module::tty;

EMU_DEFINE_FACTORIES(TtyDeviceFactory, TtyDeviceSymbolFactory, tty)
EMU_DEFINE_FACTORIES(TtyDeviceFactory, TtyDeviceSymbolFactory, default)
