find_package(Git QUIET)
if(GIT_FOUND)
execute_process(
	COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
	WORKING_DIRECTORY ${WORKINGDIR}
	OUTPUT_VARIABLE GIT_VERSION
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
else()
set(GIT_VERSION "unknown-rev")
endif()

string(TIMESTAMP CURRENT_DATE "%Y-%m-%dT%H:%M:%SZ" UTC)
string(TIMESTAMP CURRENT_TIME_T "%s" UTC)

#string(TIMESTAMP CURRENT_DATE UTC)
#set(CURRENT_DATE "11/01/2014 01:07")
configure_file(${SRC} ${DST} @ONLY)
