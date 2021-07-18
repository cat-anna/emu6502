find_program(ca65_EXECUTABLE ca65)
find_program(ld65_EXECUTABLE ld65)
message("* ca65 compiler: ${ca65_EXECUTABLE}")
message("* ca65 linker: ${ld65_EXECUTABLE}")

add_custom_target(build_all_6502_images)

function(build_6502_image_with_ca65)
  set(options)
  set(oneValueArgs SOURCE NAME LINKER_CONFIG)
  set(multiValueArgs DEPENDS)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(IMMEDIATE ${CMAKE_CURRENT_BINARY_DIR}/${ARG_NAME})
  message("* Adding image ${ARG_NAME}")

  add_custom_command(
    OUTPUT ${IMMEDIATE}.o
    COMMENT "Compiling ${ARG_NAME}"
    COMMAND ${ca65_EXECUTABLE} --verbose -l ${IMMEDIATE}.lst -o ${IMMEDIATE}.o ${ARG_SOURCE}
    DEPENDS ${ARG_SOURCE} ${ARG_DEPENDS}
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
