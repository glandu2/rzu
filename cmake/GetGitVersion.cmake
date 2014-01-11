find_package(Git QUIET)
if(GIT_FOUND)
execute_process(
     COMMAND
     ${GIT_EXECUTABLE} describe --always --dirty --abbrev=40
     WORKING_DIRECTORY ${WORKINGDIR}
     OUTPUT_VARIABLE GIT_VERSION
     OUTPUT_STRIP_TRAILING_WHITESPACE
)
else()
set(GIT_VERSION "unknown-rev")
endif()

string(TIMESTAMP CURRENT_DATE UTC)
configure_file(${SRC}.h.in ${DST}.h @ONLY)
configure_file(${SRC}.cpp.in ${DST}.cpp @ONLY)
