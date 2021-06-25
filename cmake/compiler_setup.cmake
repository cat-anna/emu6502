if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(MSVC)
  add_compile_options("/std:c++latest")
  message("* MSVC: version ${MSVC_VERSION}")
  message("* MSVC: Enabling c++latest")
else()
  # add_compile_options("-std=c++20") message(FATAL "Add support for current compiler!")
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
