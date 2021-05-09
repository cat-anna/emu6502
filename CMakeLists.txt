cmake_minimum_required(VERSION 3.18)
project(Emu6502)

# find_package(Boost REQUIRED COMPONENTS program_options) find_path(BOOST_INCLUDE_DIR Boost)
# include_directories(${BOOST_INCLUDE_DIR}) link_libraries(${Boost_LIBRARIES})

add_compile_options("/std:c++latest")
message("* MSVC: version ${MSVC_VERSION}")
# message("* MSVC: Enabling c++latest")

# find_package(cryptopp CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# find_package(xxhash CONFIG REQUIRED) find_library(XXHASH_LIBRARY xxhash) link_libraries(${XXHASH_LIBRARY})

# find_package(xxhash CONFIG REQUIRED) find_library(ZLIB_LIBRARY zlib) link_libraries(${ZLIB_LIBRARY})

add_subdirectory(src)
add_subdirectory(test/cpu)
add_subdirectory(test/assembler)
add_subdirectory(tools)
