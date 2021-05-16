set(TARGET emu_assembler)

file(GLOB_RECURSE SRC *)
add_library(${TARGET} STATIC ${SRC})

target_include_directories(${TARGET} PUBLIC ..)

target_link_libraries(${TARGET} PUBLIC fmt::fmt emu_core emu_cpu)
