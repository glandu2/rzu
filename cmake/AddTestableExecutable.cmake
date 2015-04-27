function(add_exe name sources)
  add_executable(${name} ${sources})
  # To support mixing linking in static and dynamic libraries, link each
  # library in with an extra call to target_link_libraries.
  foreach(lib "${ARGN}")
    target_link_libraries(${name} ${lib})
  endforeach()
endfunction()

function(add_lib name sources)
  add_library(${name} ${sources})
  # To support mixing linking in static and dynamic libraries, link each
  # library in with an extra call to target_link_libraries.
  foreach(lib "${ARGN}")
    target_link_libraries(${name} ${lib})
  endforeach()
endfunction()

function(add_rztest name sources)
  if(BUILD_SHARED_LIBS)
    add_definitions(-DUSING_RZTEST_SHARED)
    add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY=1)
  endif()
  include_directories("${GTEST_INCLUDE}")
  include_directories("${RZTEST_INCLUDE}")

  add_executable("${name}_test" ${sources})
  target_link_libraries("${name}_test" gtest rztest)
  add_test("${name}_test" "${EXECUTABLE_OUTPUT_PATH}/${name}_test")
endfunction()
