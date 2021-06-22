cmake_minimum_required(VERSION 3.16)

project(emu)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include("add_all_subdirectories")
include("default_targets")
include("compiler_setup")

if(NOT DEFINED TARGET_DESTINATTION)
  set(TARGET_DESTINATTION "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
endif()
message("* Target destination: " ${TARGET_DESTINATTION})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TARGET_DESTINATTION})

# set(CMAKE_CXX_CLANG_TIDY clang-tidy-11 -header-filter=.*
# -checks=*,-fuchsia*,-llvm*,-diagnostic-missing-field-initializers,-modernize-use-trailing-return-type,-google-readability-todo,-misc-unused-parameters,-misc-non-private-member-variables-in-classes,-readability-qualified-auto,-hicpp-signed-bitwise,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers,-google-readability-avoid-underscore-in-googletest-name,-cppcoreguidelines-avoid-non-const-global-variables,-google-runtime-references,-cert-err58-cpp,-cppcoreguidelines-owning-memory)

find_package(Boost REQUIRED COMPONENTS program_options)
find_path(BOOST_INCLUDE_DIR Boost)
include_directories(${BOOST_INCLUDE_DIR})
link_libraries(${Boost_LIBRARIES})

# find_package(cryptopp CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# find_package(xxhash CONFIG REQUIRED) find_library(ZLIB_LIBRARY zlib) link_libraries(${ZLIB_LIBRARY})

add_subdirectory(src)
