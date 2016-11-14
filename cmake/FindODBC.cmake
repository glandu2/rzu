# Find the ODBC driver manager includes and library.
# 
# ODBC is an open standard for connecting to different databases in a
# semi-vendor-independent fashion.  First you install the ODBC driver
# manager.  Then you need a driver for each separate database you want
# to connect to (unless a generic one works).  VTK includes neither
# the driver manager nor the vendor-specific drivers: you have to find
# those yourself.
#  
# This module defines
# ODBC_INCLUDE_DIR where to find sql.h
# ODBC_LIBRARIES, the libraries to link against to use ODBC
# ODBC_FOUND.  If false, you cannot build anything that requires MySQL.
# also defined, but not for general use is
# ODBC_LIBRARY, where to find the ODBC driver manager library.

#---For the windows platform ODBC is located automatically
if(WIN32)
	find_path(ODBC_INCLUDE_DIR sqlext.h)
	find_library(ODBC_LIBRARY odbc32)
else()
	find_path(ODBC_INCLUDE_DIR
		NAMES sqlext.h
		HINTS
		${ODBC_ROOT_DIR}/include
		PATHS
		/usr/include
		/usr/include/odbc
		/usr/local/include
		/usr/local/include/odbc
		/usr/local/odbc/include
		DOC "Specify the directory containing sql.h."
		)

	find_library(ODBC_LIBRARY
		NAMES odbc odbc32
		HINTS
		${ODBC_ROOT_DIR}/lib
		PATHS
		/usr/lib
		/usr/lib/odbc
		/usr/local/lib
		/usr/local/lib/odbc
		/usr/local/odbc/lib
		DOC "Specify the ODBC driver manager library here."
		)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ODBC DEFAULT_MSG ODBC_INCLUDE_DIR ODBC_LIBRARY)

if(ODBC_FOUND)
	add_library(ODBC UNKNOWN IMPORTED)
	set_target_properties(ODBC PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${ODBC_INCLUDE_DIR}")
	set_target_properties(ODBC PROPERTIES
		IMPORTED_LINK_INTERFACE_LANGUAGES "C"
		IMPORTED_LOCATION "${ODBC_LIBRARY}")
endif()

MARK_AS_ADVANCED(ODBC_LIBRARY ODBC_INCLUDE_DIR)
