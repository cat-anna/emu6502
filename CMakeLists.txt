cmake_minimum_required(VERSION 3.16)
project(emu)

if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(MSVC)
  add_compile_options("/std:c++latest")
  message("* MSVC: version ${MSVC_VERSION}")
  message("* MSVC: Enabling c++latest")
else()
  set(CMAKE_CXX_STANDARD 20)
  add_compile_options("-std=c++20")
  # message(FATAL "Add support for current compiler!")
endif()

if(MSVC)
  add_compile_options(/W4) # /WX
  add_compile_options(/wd4100) # unreferenced formal parameter
  add_compile_options(/wd4275) # non dll-interface class
else()
  add_compile_options(-Wall -Wextra -pedantic) # -Werror
  add_compile_options(-Wno-unused-parameter)
  add_compile_options(-Wno-missing-field-initializers)
endif()

if(WIN32)
  add_definitions(-DWINDOWS)
elseif(LINUX)
  add_definitions(-DLINUX)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("* Enabling debug features")
  add_definitions(-DDEBUG)
endif()

macro(SUBDIRLIST result curdir)
  file(
    GLOB children
    RELATIVE ${curdir}
    ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

macro(add_all_subdirecties)
  subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
  foreach(subdir ${SUBDIRS})
    add_subdirectory(${subdir})
  endforeach()
endmacro()

add_custom_target(build_all_libs)
add_custom_target(build_all_test)
add_custom_target(execute_all_test)

# set(CMAKE_CXX_CLANG_TIDY clang-tidy-11 -header-filter=.*
# -checks=*,-fuchsia*,-llvm*,-diagnostic-missing-field-initializers,-modernize-use-trailing-return-type,-google-readability-todo,-misc-unused-parameters,-misc-non-private-member-variables-in-classes,-readability-qualified-auto,-hicpp-signed-bitwise,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers,-google-readability-avoid-underscore-in-googletest-name,-cppcoreguidelines-avoid-non-const-global-variables,-google-runtime-references,-cert-err58-cpp,-cppcoreguidelines-owning-memory)

# find_package(Boost REQUIRED COMPONENTS program_options) find_path(BOOST_INCLUDE_DIR Boost)
# include_directories(${BOOST_INCLUDE_DIR}) link_libraries(${Boost_LIBRARIES})

# find_package(cryptopp CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# find_package(xxhash CONFIG REQUIRED) find_library(XXHASH_LIBRARY xxhash) link_libraries(${XXHASH_LIBRARY})

# find_package(xxhash CONFIG REQUIRED) find_library(ZLIB_LIBRARY zlib) link_libraries(${ZLIB_LIBRARY})

add_subdirectory(lib)
# add_subdirectory(src) add_subdirectory(test/cpu) add_subdirectory(test/assembler)
