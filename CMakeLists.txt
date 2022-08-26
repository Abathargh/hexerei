cmake_minimum_required(VERSION 3.22)
project(hexerei C)

set(CMAKE_C_STANDARD 99)
add_compile_options(-Wall -Wextra)
file(GLOB_RECURSE TEST_SOURCES "hexerei/*.*" "lib/list/*.*" "lib/unity/src/*.*")

add_library(hexerei hexerei/hexerei.c hexerei/record.c)
target_include_directories(hexerei PRIVATE lib/list)

set(RECORD_TEST ${TEST_SOURCES} "test/record_test.c")
add_executable(${PROJECT_NAME}_record_test ${RECORD_TEST}) 
target_include_directories(${PROJECT_NAME}_record_test PRIVATE lib/unity/src)
target_include_directories(${PROJECT_NAME}_record_test PRIVATE lib/list)
target_include_directories(${PROJECT_NAME}_record_test PRIVATE hexerei)
add_custom_target(
	test_record
	DEPENDS ${PROJECT_NAME}_record_test
	COMMAND ./${PROJECT_NAME}_record_test
)

set(HEXEREI_TEST ${TEST_SOURCES} "test/hexerei_test.c")
add_executable(${PROJECT_NAME}_hexerei_test ${HEXEREI_TEST}) 
target_include_directories(${PROJECT_NAME}_hexerei_test PRIVATE lib/unity/src)
target_include_directories(${PROJECT_NAME}_hexerei_test PRIVATE lib/list)
target_include_directories(${PROJECT_NAME}_hexerei_test PRIVATE hexerei)
add_custom_target(
	test_hexerei
	DEPENDS ${PROJECT_NAME}_hexerei_test
	COMMAND ./${PROJECT_NAME}_hexerei_test
)

add_custom_target(
	test
	DEPENDS ${PROJECT_NAME}_record_test ${PROJECT_NAME}_hexerei_test
	COMMAND ./${PROJECT_NAME}_record_test
	COMMAND ./${PROJECT_NAME}_hexerei_test 
)
