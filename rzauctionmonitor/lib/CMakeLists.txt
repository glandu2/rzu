cmake_minimum_required(VERSION 2.8.12)

file(GLOB SOURCE_FILES *.cpp *.h)

add_lib(rzauctionwriter "${SOURCE_FILES}" rzu z)
target_include_directories(rzauctionwriter PUBLIC .)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(rzauctionwriter
    PRIVATE BUILDING_LIBRZAUCTION
    INTERFACE USING_RZAUCTION_SHARED
  )
endif()
