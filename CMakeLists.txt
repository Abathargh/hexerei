cmake_minimum_required(VERSION 3.22)
project(hexerei C)

set(CMAKE_C_STANDARD 99)
add_library(hexerei hexerei/hexerei.c hexerei/record.c)


file(GLOB_RECURSE TEST_SOURCES "test/*.*" "hexerei/*.*" "lib/unity/src/*.*")
add_executable(${PROJECT_NAME}_test ${TEST_SOURCES})
target_include_directories(${PROJECT_NAME}_test PRIVATE lib/unity/src)
target_include_directories(${PROJECT_NAME}_test PRIVATE hexerei)

add_custom_target(
	test
	DEPENDS ${PROJECT_NAME}_test
	COMMAND ./${PROJECT_NAME}_test
)