cmake_minimum_required(VERSION 3.16)

project(emu)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include("add_all_subdirectories")
include("default_targets")
include("compiler_setup")
include("6502_image")

enable_testing()

if(NOT DEFINED TARGET_DESTINATTION)
  set(TARGET_DESTINATTION "${CMAKE_CURRENT_BINARY_DIR}/output")
endif()
message("* Target destination: " ${TARGET_DESTINATTION})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TARGET_DESTINATTION})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${TARGET_DESTINATTION})
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install")
set(TEST_RESULT_DIR ${CMAKE_CURRENT_BINARY_DIR}/test_result)
file(MAKE_DIRECTORY ${TEST_RESULT_DIR})

find_package(Boost REQUIRED COMPONENTS program_options filesystem)
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

if((NOT ca65_EXECUTABLE) OR (NOT ld65_EXECUTABLE))
  message("* ca65 linker or compiler are not available")
  set(HAVE_CA65 FALSE)
else()
  set(HAVE_CA65 TRUE)
endif()

add_subdirectory(src)
add_subdirectory(test)
