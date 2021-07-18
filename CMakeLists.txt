cmake_minimum_required(VERSION 3.16)

project(emu)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include("add_all_subdirectories")
include("default_targets")
include("compiler_setup")
include("6502_image")

if(NOT DEFINED TARGET_DESTINATTION)
  set(TARGET_DESTINATTION "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
endif()
message("* Target destination: " ${TARGET_DESTINATTION})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TARGET_DESTINATTION})

# set(CMAKE_CXX_CLANG_TIDY clang-tidy-11 -header-filter=.*
# -checks=*,-fuchsia*,-llvm*,-diagnostic-missing-field-initializers,-modernize-use-trailing-return-type,-google-readability-todo,-misc-unused-parameters,-misc-non-private-member-variables-in-classes,-readability-qualified-auto,-hicpp-signed-bitwise,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers,-google-readability-avoid-underscore-in-googletest-name,-cppcoreguidelines-avoid-non-const-global-variables,-google-runtime-references,-cert-err58-cpp,-cppcoreguidelines-owning-memory)

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
