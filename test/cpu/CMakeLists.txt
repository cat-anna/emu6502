set(TARGET emu_core_ut)
file(GLOB UT_SRC *.cpp)
add_executable(${TARGET} ${UT_SRC})
target_link_libraries(${TARGET} PUBLIC emu_core fmt::fmt GTest::gmock GTest::gtest fmt::fmt)
