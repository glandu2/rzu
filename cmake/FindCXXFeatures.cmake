#.rst:
# FindCXXFeatures
# ---------------
#
# Check which features of the C++ standard the compiler supports
#
# When found it will set the following variables::
#
#  CXX11_COMPILER_FLAGS                      - the compiler flags needed to get C++11 features
#
#  CXXFeatures_auto_FOUND                    - auto keyword
#  CXXFeatures_class_override_final_FOUND    - override and final keywords for classes and methods
#  CXXFeatures_constexpr_FOUND               - constexpr keyword
#  CXXFeatures_cstdint_header_FOUND          - cstdint header
#  CXXFeatures_decltype_FOUND                - decltype keyword
#  CXXFeatures_defaulted_functions_FOUND     - default keyword for functions
#  CXXFeatures_delegating_constructors_FOUND - delegating constructors
#  CXXFeatures_deleted_functions_FOUND       - delete keyword for functions
#  CXXFeatures_func_identifier_FOUND         - __func__ preprocessor constant
#  CXXFeatures_initializer_list_FOUND        - initializer list
#  CXXFeatures_lambda_FOUND                  - lambdas
#  CXXFeatures_long_long_FOUND               - long long signed & unsigned types
#  CXXFeatures_nullptr_FOUND                 - nullptr
#  CXXFeatures_rvalue_references_FOUND       - rvalue references
#  CXXFeatures_sizeof_member_FOUND           - sizeof() non-static members
#  CXXFeatures_static_assert_FOUND           - static_assert()
#  CXXFeatures_variadic_templates_FOUND      - variadic templates

#=============================================================================
# Copyright 2011-2013 Rolf Eike Beer <eike@sf-mail.de>
# Copyright 2012 Andreas Weis
# Copyright 2013 Jan KundrÃ¡t
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if (NOT CMAKE_CXX_COMPILER_LOADED)
    message(FATAL_ERROR "CXXFeatures modules only works if language CXX is enabled")
endif ()

#
### Check for needed compiler flags
#
include(CheckCXXCompilerFlag)

function(test_set_flag FLAG NAME)
    check_cxx_compiler_flag("${FLAG}" _HAS_${NAME}_FLAG)
    if (_HAS_${NAME}_FLAG)
        set(CXX11_COMPILER_FLAGS "${FLAG}" PARENT_SCOPE)
    endif ()
endfunction()

if (CMAKE_CXX_COMPILER_ID STREQUAL "XL")
    test_set_flag("-qlanglvl=extended0x" CXX0x)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "(Borland|Watcom)")
    # No C++11 flag for those compilers, but check_cxx_compiler_flag()
    # can't detect because they either will not always complain (Borland)
    # or will hang (Watcom).
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel" AND WIN32)
    # The Intel compiler on Windows may use these flags.
    test_set_flag("/Qstd=c++11" CXX11)
    if (NOT CXX11_COMPILER_FLAGS)
        test_set_flag("/Qstd=c++0x" CXX0x)
    endif ()
else ()
    test_set_flag("-std=c++11" CXX11)
    if (NOT CXX11_COMPILER_FLAGS)
        test_set_flag("-std=c++0x" CXX0x)
    endif ()
endif ()
