cmake_minimum_required (VERSION 2.6)

project(SimDB)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()
# set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

set(SOURCE
	${SOURCE}
	# src/main.cpp
)

set(HEADERS
	${HEADERS}
	src/common.h
)

add_subdirectory(src/record)
add_subdirectory(src/filesystem)
add_subdirectory(src/index)
add_subdirectory(src/system)

SET(GCC_COVERAGE_COMPILE_FLAGS "-g")

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})
ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${SOURCE} ${HEADERS})

