#

if(HAVE_CA65)
  message("* Adding 6502_functional_test")

  set(TEST_SOURCE_ORIGIN
      https://raw.githubusercontent.com/amb5l/6502_65C02_functional_tests/master/ca65/6502_functional_test.ca65)

  if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/6502_functional_test.ca65)
    message("* Using cached 6502_functional_test.ca65")
  else()
    file(DOWNLOAD ${TEST_SOURCE_ORIGIN} ${CMAKE_CURRENT_BINARY_DIR}/6502_functional_test.ca65)
    message("* Dowloaded 6502_functional_test.ca65")
  endif()

  build_6502_image_with_ca65(
    NAME
    6502_functional_test_image
    PATCH
    ${CMAKE_CURRENT_SOURCE_DIR}/patch.diff
    SOURCE
    ${CMAKE_CURRENT_BINARY_DIR}/6502_functional_test.ca65
    LINKER_CONFIG
    ${CMAKE_CURRENT_SOURCE_DIR}/linker.cfg
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/report.i65
    CONFIG
    ${CMAKE_CURRENT_SOURCE_DIR}/memory.yaml)

  define_functional_test(NAME 6502_functional_test IMAGE ${6502_functional_test_image})

endif()
