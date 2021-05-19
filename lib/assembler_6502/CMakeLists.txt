define_static_lib_with_ut(assembler_6502)
target_link_libraries(assembler_6502 PUBLIC fmt::fmt emu_core cpu_6502)
