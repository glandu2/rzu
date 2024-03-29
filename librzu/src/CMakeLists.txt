cmake_minimum_required(VERSION 3.12)

file(GLOB_RECURSE SOURCE_FILES *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h *.inl)

# Generate PacketIterator.h
function(generate_packets_includes P_FOLDER P_INCLUDE_PART P_FUNCTION_SWITCH_PART P_FUNCTION_ITERATE_PART P_SERVER_PKT_PREFIX P_CLIENT_PKT_PREFIX)
	message("Generating header includes for ${CMAKE_CURRENT_LIST_DIR}/packets/${P_FOLDER}")
	file(GLOB packet_headers RELATIVE ${CMAKE_CURRENT_LIST_DIR}/packets/${P_FOLDER} "${CMAKE_CURRENT_LIST_DIR}/packets/${P_FOLDER}/TS_*.h")

	list(SORT packet_headers)

	set(code_includes "${${P_INCLUDE_PART}}")
	foreach(header ${packet_headers})
		string(APPEND code_includes "#include \"${P_FOLDER}/${header}\"\n")
	endforeach()
	string(APPEND code_includes "\n")
	set(${P_INCLUDE_PART} ${code_includes} PARENT_SCOPE)


	set(packet_headers_origin_server ${packet_headers})
	set(packet_headers_origin_client ${packet_headers})
	list(FILTER packet_headers_origin_server EXCLUDE REGEX "TS_${P_CLIENT_PKT_PREFIX}_.*")
	list(FILTER packet_headers_origin_client EXCLUDE REGEX "TS_${P_SERVER_PKT_PREFIX}_.*")

	# TS_SC_RESULT is manually added to all session types
	list(FILTER packet_headers_origin_server EXCLUDE REGEX "TS_SC_RESULT.h$")

	set(code_function_content "${${P_FUNCTION_SWITCH_PART}}\
	if(type == SessionType::${P_FOLDER} && origin == SessionPacketOrigin::Server) {
		switch(packetType) {
			PACKET_CASE(TS_SC_RESULT);
")
	foreach(header ${packet_headers_origin_server})
		string(REPLACE ".h" "" header ${header})
		string(APPEND code_function_content "\
			PACKET_CASE(${header});
")
	endforeach()

	string(APPEND code_function_content "\
		}
	} else if(type == SessionType::${P_FOLDER} && origin == SessionPacketOrigin::Client) {
		switch(packetType) {
")
	foreach(header ${packet_headers_origin_client})
		string(REPLACE ".h" "" header ${header})
		string(APPEND code_function_content "\
			PACKET_CASE(${header});
")
	endforeach()
string(APPEND code_function_content "\
		}
	}
")
	set(${P_FUNCTION_SWITCH_PART} ${code_function_content} PARENT_SCOPE)


	set(code_iterate_content "${${P_FUNCTION_ITERATE_PART}}")
	foreach(header ${packet_headers})
		string(REPLACE ".h" "" header ${header})
		string(APPEND code_iterate_content "	PACKET_CALL(${header});\n")
	endforeach()
	string(APPEND code_iterate_content "\n")
	set(${P_FUNCTION_ITERATE_PART} ${code_iterate_content} PARENT_SCOPE)
endfunction()

generate_packets_includes("AuthClient" INCLUDE_PART FUNCTION_SWITCH_PART FUNCTION_ITERATE_PART "AC" "CA")
generate_packets_includes("GameClient" INCLUDE_PART FUNCTION_SWITCH_PART FUNCTION_ITERATE_PART "SC" "CS")
generate_packets_includes("UploadClient" INCLUDE_PART FUNCTION_SWITCH_PART FUNCTION_ITERATE_PART "UC" "CU")

configure_file(packets/PacketIterator.h.in packets/PacketIterator.h @ONLY NEWLINE_STYLE UNIX)
list(APPEND HEADER_FILES packets/PacketIterator.h.in)
list(APPEND HEADER_FILES ${CMAKE_CURRENT_BINARY_DIR}/packets/PacketIterator.h)

# Generate PacketMetadata.cpp


include(AddVersionInfo OPTIONAL RESULT_VARIABLE versioncmakefound)
if( versioncmakefound )
	add_git_version_info(${TARGET_NAME}_GitVersion SOURCE_FILES)
endif()


find_package(OpenSSL REQUIRED)
find_package(ODBC REQUIRED)
set(LIBS uv ${OPENSSL_LIBRARIES} z iconv ${ODBC_LIBRARY} CURL::libcurl)
if(WIN32)
	list(APPEND LIBS dbghelp)
endif()

add_shared_lib(${TARGET_NAME} "${SOURCE_FILES};${HEADER_FILES}" "${LIBS}")
target_include_directories(${TARGET_NAME}
	PUBLIC lib packets ${CMAKE_CURRENT_BINARY_DIR}/packets ${ODBC_INCLUDE_DIR}
	PRIVATE ${OPENSSL_INCLUDE_DIR}
)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	target_compile_definitions(${TARGET_NAME} PUBLIC NOMINMAX)
endif()

target_compile_definitions(${TARGET_NAME}
	PRIVATE BUILDING_LIBRZU
	INTERFACE USING_RZU_SHARED
)

# Install libcurl and OpenSSL DLLs on Windows
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
		set(DLL_LIB_SUFFIX "-x64")
	else()
		set(DLL_LIB_SUFFIX "")
	endif()

	# For OpenSSL 3.0+, OPENSSL_VERSION_* variables are not set
	if(NOT DEFINED OPENSSL_VERSION_MAJOR)
		string(REGEX MATCHALL "([0-9])+" OPENSSL_VERSION_NUMBER "${OPENSSL_VERSION}")
		list(POP_FRONT OPENSSL_VERSION_NUMBER
			OPENSSL_VERSION_MAJOR
			OPENSSL_VERSION_MINOR
			OPENSSL_VERSION_FIX)
	endif()

	find_file(OPENSSL_LIBEAY32_DLL
		NAMES
		"libeay32.dll"
		"libcrypto-${OPENSSL_VERSION_MAJOR}_${OPENSSL_VERSION_MINOR}${DLL_LIB_SUFFIX}.dll"
		"libcrypto-${OPENSSL_VERSION_MAJOR}${DLL_LIB_SUFFIX}.dll"
		HINTS
		${OPENSSL_INCLUDE_DIR}/../bin
		NO_DEFAULT_PATH)

	if(OPENSSL_LIBEAY32_DLL)
		message(STATUS "Will install ${OPENSSL_LIBEAY32_DLL}")
		install(PROGRAMS ${OPENSSL_LIBEAY32_DLL} DESTINATION ./ COMPONENT "deps")
	else()
		message(STATUS "libcrypto-${OPENSSL_VERSION_MAJOR}${DLL_LIB_SUFFIX}.dll won't be installed")
	endif()

	find_file(OPENSSL_LIBSSL_DLL
		NAMES
		"libssl-${OPENSSL_VERSION_MAJOR}_${OPENSSL_VERSION_MINOR}${DLL_LIB_SUFFIX}.dll"
		"libssl-${OPENSSL_VERSION_MAJOR}${DLL_LIB_SUFFIX}.dll"
		HINTS
		${OPENSSL_INCLUDE_DIR}/../bin
		NO_DEFAULT_PATH)

	if(OPENSSL_LIBSSL_DLL)
		message(STATUS "Will install ${OPENSSL_LIBSSL_DLL}")
		install(PROGRAMS ${OPENSSL_LIBSSL_DLL} DESTINATION ./ COMPONENT "deps")
	else()
		message(STATUS "libssl-${OPENSSL_VERSION_MAJOR}${DLL_LIB_SUFFIX}.dll won't be installed")
	endif()
endif()

add_subdirectory(packet_preprocessing_targets)
