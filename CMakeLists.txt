cmake_minimum_required(VERSION 3.16)
project(Emu6502)

if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(MSVC)
  add_compile_options("/std:c++latest")
  message("* MSVC: version ${MSVC_VERSION}")
  message("* MSVC: Enabling c++latest")
else()
  set(CMAKE_CXX_STANDARD 20)
  # add_compile_options("-std=c++20")
  # message(FATAL "Add support for current compiler!")
endif()

if(WIN32)
  add_definitions(-DWINDOWS)
elseif(LINUX)
  add_definitions(-DLINUX)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DDEBUG)
endif()

# set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*)

# find_package(Boost REQUIRED COMPONENTS program_options) find_path(BOOST_INCLUDE_DIR Boost)
# include_directories(${BOOST_INCLUDE_DIR}) link_libraries(${Boost_LIBRARIES})

# find_package(cryptopp CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# find_package(xxhash CONFIG REQUIRED) find_library(XXHASH_LIBRARY xxhash) link_libraries(${XXHASH_LIBRARY})

# find_package(xxhash CONFIG REQUIRED) find_library(ZLIB_LIBRARY zlib) link_libraries(${ZLIB_LIBRARY})

add_subdirectory(src)
add_subdirectory(test/cpu)
add_subdirectory(test/assembler)
add_subdirectory(tools)
