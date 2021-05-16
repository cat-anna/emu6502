set(TARGET emu_cpu)

file(GLOB_RECURSE SRC *)
add_library(${TARGET} STATIC ${SRC})

target_include_directories(${TARGET} PUBLIC ..)
target_link_libraries(${TARGET} PUBLIC fmt::fmt emu_core)
