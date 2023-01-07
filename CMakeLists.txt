cmake_minimum_required (VERSION 2.6)

project(SimDB)

SET(CMAKE_BUILD_TYPE "Debug")

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

IF (CMAKE_BUILD_TYPE EQUAL Debug)
	SET(GCC_COVERAGE_COMPILE_FLAGS "-g")
ELSE()
	SET(GCC_COVERAGE_COMPILE_FLAGS "-O3")
ENDIF()
add_definitions(${GCC_COVERAGE_COMPILE_FLAG})
# add_definitions(-DNO_OPTIM)

SET(GCC_COVERAGE_LINK_FLAGS    "-pthread")
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
SET(ANTLR4_RUNTIME_DIRECTORY "/usr/local/include/antlr4-runtime")
SET(ANTLR4_RUNTIME_SHARED_LIBRARY "/usr/local/lib/libantlr4-runtime.so")

SET(HEADERS
	${HEADERS}
	src/common.h
)

SET(SOURCE
	${SOURCE}
	src/main.cpp
)

add_subdirectory(src/record)
add_subdirectory(src/filesystem)
add_subdirectory(src/index)
add_subdirectory(src/system)
add_subdirectory(src/parser)

include_directories(${ANTLR4_RUNTIME_DIRECTORY})
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE} ${HEADERS})
add_library(libantlr4-runtime SHARED IMPORTED)
set_property(TARGET libantlr4-runtime PROPERTY IMPORTED_LOCATION ${ANTLR4_RUNTIME_SHARED_LIBRARY})
target_link_libraries(${CMAKE_PROJECT_NAME} libantlr4-runtime ${GCC_COVERAGE_LINK_FLAGS})
