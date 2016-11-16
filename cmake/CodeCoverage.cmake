# Copyright (c) 2012 - 2015, Lars Bilke
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#
#
# 2012-01-31, Lars Bilke
# - Enable Code Coverage
#
# 2013-09-17, Joakim Söderberg
# - Added support for Clang.
# - Some additional usage instructions.
#
# USAGE:

# 0. (Mac only) If you use Xcode 5.1 make sure to patch geninfo as described here:
#      http://stackoverflow.com/a/22404544/80480
#
# 1. Copy this file into your cmake modules path.
#
# 2. Add the following line to your CMakeLists.txt:
#      INCLUDE(CodeCoverage)
#
# 3. Set compiler flags to turn off optimization and enable coverage:
#    SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
#	 SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
#
# 3. Use the function SETUP_TARGET_FOR_COVERAGE to create a custom make target
#    which runs your test executable and produces a lcov code coverage report:
#    Example:
#	 SETUP_TARGET_FOR_COVERAGE(
#				my_coverage_target  # Name for custom target.
#				test_driver         # Name of the test driver executable that runs the tests.
#									# NOTE! This should always have a ZERO as exit code
#									# otherwise the coverage generation will not complete.
#				coverage            # Name of output directory.
#				)
#
# 4. Build a Debug build:
#	 cmake -DCMAKE_BUILD_TYPE=Debug ..
#	 make
#	 make my_coverage_target
#
#

# Check prereqs
get_filename_component(COMPILER_DIR ${CMAKE_C_COMPILER} DIRECTORY)

FIND_PROGRAM( GCOV_PATH gcov gcov.exe gcov.bat HINTS ${COMPILER_DIR})
FIND_PROGRAM( LCOV_PATH lcov lcov.bat lcov.perl lcov.pl HINTS ${COMPILER_DIR})

get_filename_component(LCOV_DIR ${LCOV_PATH} DIRECTORY)
FIND_PROGRAM( GENHTML_PATH genhtml genhtml.bat genhtml.perl genhtml.pl HINTS ${LCOV_DIR})

IF(NOT CMAKE_COMPILER_IS_GNUCXX)
	# Clang version 3.0.0 and greater now supports gcov as well.
	MESSAGE(WARNING "Compiler is not GNU gcc! Clang Version 3.0.0 and greater supports gcov as well, but older versions don't.")
ENDIF() # NOT CMAKE_COMPILER_IS_GNUCXX

SET(CMAKE_CXX_FLAGS_COVERAGE
    "${CMAKE_CXX_FLAGS_DEBUG} --coverage -fprofile-arcs -ftest-coverage"
    CACHE STRING "Flags used by the C++ compiler during coverage builds.")
SET(CMAKE_C_FLAGS_COVERAGE
    "${CMAKE_C_FLAGS_DEBUG} --coverage -fprofile-arcs -ftest-coverage"
    CACHE STRING "Flags used by the C compiler during coverage builds.")
SET(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage -fprofile-arcs -ftest-coverage"
    CACHE STRING "Flags used for linking binaries during coverage builds.")
SET(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} --coverage -fprofile-arcs -ftest-coverage"
    CACHE STRING "Flags used by the shared libraries linker during coverage builds.")
SET(CMAKE_MODULE_LINKER_FLAGS_COVERAGE
    "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} --coverage -fprofile-arcs -ftest-coverage"
    CACHE STRING "Flags used by the module libraries linker during coverage builds.")
SET(CMAKE_STATIC_LINKER_FLAGS_COVERAGE
    "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} --coverage -fprofile-arcs -ftest-coverage"
    CACHE STRING "Flags used by the module libraries linker during coverage builds.")
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_COVERAGE
    CMAKE_C_FLAGS_COVERAGE
    CMAKE_EXE_LINKER_FLAGS_COVERAGE
	CMAKE_SHARED_LINKER_FLAGS_COVERAGE
	CMAKE_MODULE_LINKER_FLAGS_COVERAGE
	CMAKE_STATIC_LINKER_FLAGS_COVERAGE)

if(EXISTS ${LCOV_PATH} AND EXISTS ${GENHTML_PATH})
	message("Adding coverage target")
	add_custom_target(coverage
		# Cleanup lcov
		#perl.exe   ${LCOV_PATH} --directory . --zerocounters

		# Capturing lcov counters and generating report
		COMMAND ${CMAKE_COMMAND} -E remove coverage.info coverage.info.cleaned
		COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info --gcov-tool ${GCOV_PATH}
		COMMAND ${LCOV_PATH} --remove coverage.info "*/mingw64/*" "*/mingw/*" "/usr/*" "*/zlib/*" "*/libuv/*" "*/gtest/*" "*/libiconv/*" "*/rztest/*" "*/test/*" "lib/aliases.gperf" --output-file coverage.info.cleaned --rc lcov_branch_coverage=1
		COMMAND ${GENHTML_PATH} -o coverage_html coverage.info.cleaned --rc genhtml_branch_coverage=1

		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMENT "Processing code coverage counters and generating report."
		VERBATIM
		)
	add_custom_target(coverage_clean
		# Cleanup lcov
		COMMAND ${LCOV_PATH} --directory . --zerocounters
		COMMAND ${CMAKE_COMMAND} -E remove_directory coverage_html

		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMENT "Resetting code coverage counters to zero."
		VERBATIM
		)
endif()

