define_executable(emu_packager)
target_link_libraries(${TARGET} PUBLIC emu_core)
target_link_libraries(${TARGET} PUBLIC Boost::program_options)
