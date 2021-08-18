set(MEM_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/memory.yaml)

file(GLOB test_src *.asm)

foreach(src ${test_src})
  get_filename_component(name ${src} NAME_WE)
  set(image_target ${name}_image)
  build_6502_image(NAME ${image_target} SOURCE ${src} CONFIG ${MEM_CONFIG})
  define_functional_test(NAME ${name} IMAGE ${${image_target}})
endforeach()
