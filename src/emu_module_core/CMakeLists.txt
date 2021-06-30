define_static_lib_with_ut(emu_module_core)
target_link_libraries(${TARGET} PUBLIC emu_core Boost::filesystem)
