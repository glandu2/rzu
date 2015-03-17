function(add_exe name sources)
  add_executable(${name} ${sources})
  # To support mixing linking in static and dynamic libraries, link each
  # library in with an extra call to target_link_libraries.
  foreach(lib "${ARGN}")
    target_link_libraries(${name} ${lib})
  endforeach()
  install(TARGETS ${name} DESTINATION ./)
endfunction()

function(add_lib name sources)
  add_library(${name} ${sources})
  # To support mixing linking in static and dynamic libraries, link each
  # library in with an extra call to target_link_libraries.
  foreach(lib "${ARGN}")
    target_link_libraries(${name} ${lib})
  endforeach()
  install(TARGETS ${name} DESTINATION ./)
endfunction()

function(add_rztest name sources)
  include_directories("${GTEST_INCLUDE}")
  include_directories("${RZTEST_INCLUDE}")

  add_executable("${name}_test" ${sources})
  target_link_libraries("${name}_test" gtest rztest)
  add_test("${name}_test" "${EXECUTABLE_OUTPUT_PATH}/${name}_test")
endfunction()
