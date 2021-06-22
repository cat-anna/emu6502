define_executable(emu_6502_ac)

target_link_libraries(${TARGET} PUBLIC emu_core)
target_link_libraries(${TARGET} PUBLIC emu_6502)
