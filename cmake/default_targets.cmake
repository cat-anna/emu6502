add_custom_target(build_all_libs)
add_custom_target(build_all_test)
add_custom_target(build_all_executables)
add_custom_target(build_all_modules)
add_custom_target(execute_all_test)

function(define_static_lib target_name)
  file(GLOB_RECURSE SRC src/*.cpp src/*.hpp include/*.hpp)

  message("* Adding static lib ${target_name}")
  add_library(${target_name} STATIC ${SRC})
  target_include_directories(${target_name} PUBLIC include)
  target_include_directories(${target_name} PRIVATE src)
  target_link_libraries(${target_name} PUBLIC fmt::fmt)
  add_dependencies(build_all_libs ${target_name})

  set(TARGET
      ${target_name}
      PARENT_SCOPE)
  set(LIB_TARGET
      ${target_name}
      PARENT_SCOPE)
endfunction()

function(define_module target_name)
  set(MODULE_NAME "emu.module.${target_name}")
  set(LIB_NAME "emu_${target_name}")

  define_static_lib(${LIB_NAME})
  file(GLOB_RECURSE SRC module/*)
  # TARGET_DESTINATTION
  message("* Adding module ${MODULE_NAME}")
  add_library(${MODULE_NAME} SHARED ${SRC})
  target_include_directories(${MODULE_NAME} PUBLIC include)
  target_include_directories(${MODULE_NAME} PRIVATE src)
  target_link_libraries(${MODULE_NAME} PUBLIC fmt::fmt ${LIB_NAME})
  add_dependencies(build_all_modules ${MODULE_NAME})

  set(TARGET
      ${LIB_NAME}
      PARENT_SCOPE)
  set(MODULE_TARGET
      ${MODULE_NAME}
      PARENT_SCOPE)
  set(LIB_TARGET
      ${LIB_NAME}
      PARENT_SCOPE)
endfunction()

function(define_executable target_name)
  file(GLOB_RECURSE SRC src/*.cpp src/*.hpp include/*.hpp)

  message("* Adding executable ${target_name}")
  add_executable(${target_name} ${SRC})
  target_include_directories(${target_name} PUBLIC include)
  target_include_directories(${target_name} PRIVATE src)
  target_link_libraries(${target_name} PUBLIC fmt::fmt)
  add_dependencies(build_all_executables ${target_name})

  set(TARGET
      ${target_name}
      PARENT_SCOPE)
endfunction()

function(define_ut_target target_name ut_name)
  file(GLOB src_ut ${ut_name}/*)

  string(REGEX REPLACE "test(.*)" "\\1" short_ut_name ${ut_name})
  string(REPLACE "/" "_" valid_ut_name "${short_ut_name}")

  set(target_ut_name "${target_name}${valid_ut_name}_ut")
  message("* Adding UTs ${target_ut_name} ")

  add_executable(${target_ut_name} ${src_ut})
  target_include_directories(${target_ut_name} PRIVATE src test)
  target_link_libraries(${target_ut_name} PUBLIC ${target_name} fmt::fmt GTest::gmock GTest::gtest ${ut_runner})
  target_compile_definitions(${target_ut_name} PRIVATE -DWANTS_GTEST_MOCKS)

  add_custom_target(
    run_${target_ut_name}
    COMMAND ${target_ut_name}
    WORKING_DIRECTORY ${TARGET_DESTINATTION}
    COMMENT "Running test ${target_ut_name}"
    DEPENDS ${target_ut_name} ${target_name})

  add_dependencies(execute_all_test run_${target_ut_name})
  add_dependencies(build_all_test ${target_ut_name})
endfunction()

function(define_ut_multi_target target_name ut_name)
  file(
    GLOB src_ut
    LIST_DIRECTORIES true
    ${ut_name}/*)

  set(srclist "")

  foreach(child ${src_ut})
    file(RELATIVE_PATH rel_child ${CMAKE_CURRENT_SOURCE_DIR} ${child})
    if(IS_DIRECTORY ${child})
      define_ut_target(${target_name} ${rel_child})
    else()
      list(APPEND srclist ${rel_child})
    endif()
  endforeach()
  if(srclist)
    define_ut_target(${target_name} ${ut_name})
  endif()
endfunction()

function(define_static_lib_with_ut target_name)
  define_static_lib(${target_name})
  define_ut_multi_target(${target_name} test)
  set(TARGET
      ${target_name}
      PARENT_SCOPE)
endfunction()

function(define_module_with_ut target_name)
  define_module(${target_name})
  define_ut_multi_target(${LIB_TARGET} test)

  set(TARGET
      ${LIB_TARGET}
      PARENT_SCOPE)
endfunction()
