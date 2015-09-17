function(add_exe name sources)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/metadata.rc.in")
    file(GLOB ICON_FILES "../res/*.ico")
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/metadata.rc"
        COMMAND ${CMAKE_COMMAND} -D SRC="${CMAKE_CURRENT_SOURCE_DIR}/metadata.rc.in"
            -D DST="${CMAKE_CURRENT_BINARY_DIR}/metadata.rc"
            -D TARGET_NAME=${TARGET_NAME}
            -D WORKINGDIR=${CMAKE_CURRENT_SOURCE_DIR}
            -P ${CUSTOM_CMAKE_MODULES_PATH}/GetGitVersion.cmake
        DEPENDS ${ICON_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/metadata.rc.in" ${CUSTOM_CMAKE_MODULES_PATH}/GetGitVersion.cmake
    )
    list(APPEND sources "${CMAKE_CURRENT_BINARY_DIR}/metadata.rc" "${CMAKE_CURRENT_SOURCE_DIR}/metadata.rc.in")
  endif()
  add_executable(${name} ${sources})
  # To support mixing linking in static and dynamic libraries, link each
  # library in with an extra call to target_link_libraries.
  foreach(lib "${ARGN}")
    target_link_libraries(${name} ${lib})
  endforeach()

  install(TARGETS ${name} RUNTIME DESTINATION ./ COMPONENT "binaries")
  install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${name}.pdb DESTINATION ./symbols/ COMPONENT "symbols")
endfunction()

function(add_lib name sources)
  add_library(${name} ${sources})
  # To support mixing linking in static and dynamic libraries, link each
  # library in with an extra call to target_link_libraries.
  foreach(lib "${ARGN}")
    target_link_libraries(${name} ${lib})
  endforeach()

  if(BUILD_SHARED_LIBS)
    install(TARGETS ${name}
      RUNTIME DESTINATION ./
      LIBRARY DESTINATION ./
      COMPONENT "binaries"
    )
    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${name}.pdb DESTINATION ./symbols/ COMPONENT "symbols")
  endif()
endfunction()

function(add_rztest name sources)
  if(BUILD_SHARED_LIBS)
    add_definitions(-DUSING_RZTEST_SHARED)
    add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY=1)
  endif()
  include_directories("${GTEST_INCLUDE}")
  include_directories("${RZTEST_INCLUDE}")

  add_executable("${name}_test" ${sources})
  target_link_libraries("${name}_test" rztest)
  add_test("${name}_test" "${EXECUTABLE_OUTPUT_PATH}/${name}_test")
endfunction()
