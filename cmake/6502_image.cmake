find_program(ca65_EXECUTABLE ca65)
find_program(ld65_EXECUTABLE ld65)
find_program(git_executable git)

message("* ca65 compiler: ${ca65_EXECUTABLE}")
message("* ca65 linker: ${ld65_EXECUTABLE}")

add_custom_target(build_all_6502_images ALL)
add_dependencies(build_all_6502_images build_all_modules)

function(build_6502_image_with_ca65)
  set(options)
  set(oneValueArgs SOURCE NAME LINKER_CONFIG PATCH CONFIG)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(IMMEDIATE ${CMAKE_CURRENT_BINARY_DIR}/${ARG_NAME})
  set(TARGET_IMAGE ${TARGET_DESTINATTION}/${ARG_NAME}.emu_image)
  message("* Adding image ${ARG_NAME}")

  get_filename_component(SRC_NAME ${ARG_SOURCE} NAME)

  if(DEFINED ARG_PATCH)
    set(PATCH_DIR ${IMMEDIATE}/patch)
    set(COMPILE_SOURCE ${PATCH_DIR}/${SRC_NAME})
    set(PATCHED_SOURCE ${COMPILE_SOURCE})

    file(RELATIVE_PATH PATCH_RELATIVE ${PROJECT_SOURCE_DIR} ${PATCH_DIR})
    add_custom_command(
      OUTPUT ${COMPILE_SOURCE}
      COMMENT "Patching ${ARG_NAME}"
      COMMAND ${CMAKE_COMMAND} -E copy ${ARG_SOURCE} ${PATCH_DIR}
      COMMAND ${git_executable} apply --no-index --directory=${PATCH_RELATIVE} ${ARG_PATCH}
      DEPENDS ${ARG_SOURCE} ${ARG_PATCH}
      WORKING_DIRECTORY ${PATCH_DIR}
      VERBATIM)
  else()
    set(COMPILE_SOURCE ${ARG_SOURCE})
  endif()

  add_custom_command(
    OUTPUT ${IMMEDIATE}.o
    COMMENT "Compiling ${ARG_NAME}"
    COMMAND ${ca65_EXECUTABLE} --include-dir ${CMAKE_CURRENT_SOURCE_DIR} --verbose -l ${IMMEDIATE}.lst -o ${IMMEDIATE}.o
            ${COMPILE_SOURCE}
    DEPENDS ${ARG_SOURCE} ${ARG_DEPENDS} ${ARG_PATCH} ${COMPILE_SOURCE}
    VERBATIM)

  add_custom_command(
    OUTPUT ${IMMEDIATE}.bin
    COMMENT "Linking ${ARG_NAME}"
    COMMAND ${ld65_EXECUTABLE} ${IMMEDIATE}.o -o ${IMMEDIATE}.bin -m ${IMMEDIATE}.map -C ${ARG_LINKER_CONFIG}
    DEPENDS ${IMMEDIATE}.o ${ARG_LINKER_CONFIG}
    VERBATIM)

  add_custom_command(
    OUTPUT ${TARGET_IMAGE}
    COMMENT "Packing ${ARG_NAME}"
    COMMAND emu_packager --config ${ARG_CONFIG} --output ${TARGET_IMAGE} --override image_file=${IMMEDIATE}.bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${IMMEDIATE}.bin ${ARG_CONFIG} emu_packager
    VERBATIM)

  add_custom_target(${ARG_NAME} DEPENDS ${TARGET_IMAGE})
  add_dependencies(build_all_6502_images ${ARG_NAME})

  set_property(
    TARGET ${ARG_NAME}
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES
             ${PATCHED_SOURCE}
             ${IMMEDIATE}.bin
             ${IMMEDIATE}.o
             ${IMMEDIATE}.lst
             ${IMMEDIATE}.map
             ${TEST_IMAGE})

  set(${ARG_NAME}
      ${TARGET_IMAGE}
      PARENT_SCOPE)
endfunction()

function(build_6502_image)
  set(options)
  set(oneValueArgs SOURCE NAME CONFIG)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(IMMEDIATE ${CMAKE_CURRENT_BINARY_DIR}/${ARG_NAME})
  set(TARGET_IMAGE ${TARGET_DESTINATTION}/${ARG_NAME}.emu_image)
  message("* Adding image ${ARG_NAME}")

  add_custom_command(
    OUTPUT ${IMMEDIATE}.bin
    COMMENT "Compiling ${ARG_NAME}"
    COMMAND emu_6502_ac --config ${ARG_CONFIG} --input ${ARG_SOURCE} --hex-dump ${IMMEDIATE}.hex --bin-output
            ${IMMEDIATE}.bin
    DEPENDS ${ARG_SOURCE} ${ARG_DEPENDS} emu_6502_ac build_all_modules
    VERBATIM)

  add_custom_command(
    OUTPUT ${TARGET_IMAGE}
    COMMENT "Packing ${ARG_NAME}"
    COMMAND emu_packager --config ${ARG_CONFIG} --output ${TARGET_IMAGE} --override image_file=${IMMEDIATE}.bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${ARG_SOURCE} ${ARG_DEPENDS} ${IMMEDIATE}.bin emu_packager
    VERBATIM)

  add_custom_target(${ARG_NAME} DEPENDS build_all_modules ${TARGET_IMAGE})
  add_dependencies(build_all_6502_images ${ARG_NAME})

  set_property(
    TARGET ${ARG_NAME}
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES ${IMMEDIATE}.bin ${TEST_IMAGE})

  set(${ARG_NAME}
      ${TARGET_IMAGE}
      PARENT_SCOPE)

endfunction()
