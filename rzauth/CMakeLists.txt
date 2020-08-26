cmake_minimum_required(VERSION 2.8.12)
project(rzauth)

set(TARGET_NAME rzauth)

add_subdirectory(src)
add_subdirectory(test)

install(
	FILES README.md
	DESTINATION ./
	RENAME ${TARGET_NAME}_user_guide.md
)
