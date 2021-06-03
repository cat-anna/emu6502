function(define_static_lib target_name)

  file(GLOB_RECURSE SRC src/*.cpp src/*.hpp include/*.hpp)

  add_library(${target_name} STATIC ${SRC})
  target_include_directories(${target_name} PUBLIC include)
  target_include_directories(${target_name} PRIVATE src)
  target_link_libraries(${target_name} PUBLIC fmt::fmt)
  add_dependencies(build_all_libs ${target_name})

  set(TARGET
      ${target_name}
      PARENT_SCOPE)
endfunction()

function(define_ut_target target_name ut_target_name target_src)
  add_executable(${ut_target_name} ${target_src})
  target_include_directories(${ut_target_name} PRIVATE src test)
  target_link_libraries(${ut_target_name} PUBLIC ${target_name} fmt::fmt)
  add_dependencies(build_all_test ${ut_target_name})
  add_custom_target(
    run_${ut_target_name}
    COMMAND ${ut_target_name}
    WORKING_DIRECTORY ${TARGET_DESTINATTION}
    COMMENT "Running test ${ut_target_name}"
    DEPENDS ${ut_target_name} ${target_name})
  add_dependencies(execute_all_test run_${ut_target_name})
endfunction()

function(define_static_lib_with_ut target_name)
  define_static_lib(${target_name})

  file(GLOB_RECURSE gtest_src test/*)
  if(gtest_src)
    message("* Adding UTs for ${target_name}")
    set(ut_target ${target_name}_ut)
    define_ut_target(${target_name} ${ut_target} "${gtest_src}")
    target_link_libraries(${ut_target} PUBLIC GTest::gmock GTest::gtest) # gtest_main
    target_compile_definitions(${ut_target} PRIVATE -DWANTS_GTEST_MOCKS)
  endif()

  set(TARGET
      ${target_name}
      PARENT_SCOPE)
  set(TARGET_UT
      ${ut_target}
      PARENT_SCOPE)
endfunction()

add_all_subdirecties()
