define_executable(emu_6502_runner)

target_link_libraries(${TARGET} PUBLIC emu_core emu_module_core emu_simulation)
target_link_libraries(${TARGET} PUBLIC Boost::program_options)
