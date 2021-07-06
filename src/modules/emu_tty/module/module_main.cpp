#include "emu_core/plugins/plugin.hpp"
#include "emu_tty/tty_device_factory.hpp"
#include "emu_tty/tty_symbol_factory.hpp"

using namespace emu::tty;

EMU_DEFINE_FACTORIES(TtyDeviceFactory, TtyDeviceSymbolFactory, tty)
EMU_DEFINE_FACTORIES(TtyDeviceFactory, TtyDeviceSymbolFactory, default)
