cmake_minimum_required (VERSION 2.6)

project(SimDB)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()
# set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

add_subdirectory(src/record)
add_subdirectory(src/filesystem)
add_subdirectory(src/index)
add_subdirectory(src/parser)

set(HEADERS
	${HEADERS}
	src/common.h
)

set(SOURCE
	${SOURCE}
	# src/main.cpp
)

SET(GCC_COVERAGE_COMPILE_FLAGS "-g")
SET(GCC_COVERAGE_LINK_FLAGS    "-pthread")

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})
include_directories("/usr/local/include/antlr4-runtime")
ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${SOURCE} ${HEADERS})
# target_link_libraries(${CMAKE_PROJECT_NAME} /usr/local/lib/libantlr4-runtime.a)
add_library(libantlr4-runtime SHARED IMPORTED)
set_property(TARGET libantlr4-runtime PROPERTY IMPORTED_LOCATION "/usr/local/lib/libantlr4-runtime.so")
target_link_libraries(${CMAKE_PROJECT_NAME} libantlr4-runtime ${GCC_COVERAGE_LINK_FLAGS})
