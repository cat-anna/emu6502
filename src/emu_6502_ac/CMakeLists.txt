define_executable(emu_6502_ac)

target_link_libraries(${TARGET} PUBLIC emu_core emu_module_core emu_6502)
target_link_libraries(${TARGET} PUBLIC Boost::program_options)
