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

if(WIN32)
	execute_process(
		COMMAND "cmd" "/C" "(for /f " tokens=1,2 delims== " %x in ('wmic path win32_utctime get year^,month^,day^,hour^,minute^,second /format:list ^| findstr ^=') do @set %x=0%y) && call echo %year:~-5,4%-%month:~-3,2%-%day:~-3,2%T%Hour:~-3,2%:%Minute:~-3,2%:%Second:~-3,2%Z"
		OUTPUT_VARIABLE CURRENT_DATE
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
elseif(UNIX)
	execute_process(
		COMMAND "date" "-u" "+%Y-%m-%dT%H:%M:%SZ"
		OUTPUT_VARIABLE CURRENT_DATE
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
else()
	message(SEND_ERROR "date not implemented")
	set(${CURRENT_DATE} 000000000000)
endif()

#string(TIMESTAMP CURRENT_DATE UTC)
#set(CURRENT_DATE "11/01/2014 01:07")
configure_file(${SRC} ${DST} @ONLY)
