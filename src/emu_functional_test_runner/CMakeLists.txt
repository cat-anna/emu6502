define_executable(emu_functional_test_runner_ut)
target_link_libraries(${TARGET} PUBLIC emu_core emu_simulation emu_module_core)
target_link_libraries(${TARGET} PUBLIC fmt::fmt GTest::gmock GTest::gtest ${ut_runner})

target_compile_definitions(${TARGET} PRIVATE -DWANTS_GTEST_MOCKS)

disable_clang_tidy(${TARGET})
