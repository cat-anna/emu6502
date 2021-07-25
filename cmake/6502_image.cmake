find_program(ca65_EXECUTABLE ca65)
find_program(ld65_EXECUTABLE ld65)
find_program(git_executable git)

message("* ca65 compiler: ${ca65_EXECUTABLE}")
message("* ca65 linker: ${ld65_EXECUTABLE}")

add_custom_target(build_all_6502_images)

function(build_6502_image_with_ca65)
  set(options)
  set(oneValueArgs SOURCE NAME LINKER_CONFIG PATCH)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(IMMEDIATE ${CMAKE_CURRENT_BINARY_DIR}/${ARG_NAME})
  message("* Adding image ${ARG_NAME}")

  get_filename_component(SRC_NAME ${ARG_SOURCE} NAME)

  if(DEFINED ARG_PATCH)
    set(PATCH_DIR ${IMMEDIATE}/patch)
    set(COMPILE_SOURCE ${PATCH_DIR}/${SRC_NAME})

    add_custom_command(
      OUTPUT ${COMPILE_SOURCE}
      COMMENT "Patching ${ARG_NAME}"
      COMMAND ${CMAKE_COMMAND} -E copy ${ARG_SOURCE} ${PATCH_DIR}
      COMMAND ${git_executable} apply --no-index --directory=${PATCH_DIR} ${ARG_PATCH}
      DEPENDS ${ARG_SOURCE} ${ARG_PATCH}
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

  add_custom_target(${ARG_NAME} DEPENDS ${IMMEDIATE}.bin)
  set(${ARG_NAME}
      ${IMMEDIATE}.bin
      PARENT_SCOPE)

  add_dependencies(build_all_6502_images ${ARG_NAME})

endfunction()

function(build_6502_image)
  set(options)
  set(oneValueArgs SOURCE NAME CONFIG)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(IMMEDIATE ${CMAKE_CURRENT_BINARY_DIR}/${ARG_NAME})
  message("* Adding image ${ARG_NAME}")

  add_custom_command(
    OUTPUT ${IMMEDIATE}.bin
    COMMENT "Compiling ${ARG_NAME}"
    COMMAND emu_6502_ac -v --config ${ARG_CONFIG} --input ${ARG_SOURCE} --hex-dump ${IMMEDIATE}.hex --bin-output
            ${IMMEDIATE}.bin
    DEPENDS ${ARG_SOURCE} ${ARG_DEPENDS} emu_6502_ac
    VERBATIM)

  add_custom_target(${ARG_NAME} DEPENDS ${IMMEDIATE}.bin)

  set(${ARG_NAME}
      ${IMMEDIATE}.bin
      PARENT_SCOPE)

  add_dependencies(build_all_6502_images ${ARG_NAME})

endfunction()
