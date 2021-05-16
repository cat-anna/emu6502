set(TARGET emu_assembler_ut)
file(GLOB UT_SRC *.cpp)
add_executable(${TARGET} ${UT_SRC})
target_link_libraries(${TARGET} PUBLIC emu_assembler emu_core emu_cpu fmt::fmt GTest::gmock GTest::gtest fmt::fmt)
