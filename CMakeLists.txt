cmake_minimum_required (VERSION 2.6)

project(SimDB)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BINARY_DIR build)
set(EXECUTABLE_OUTPUT_PATH build)
set(SOURCE
	${SOURCE}
	src/record/FileHandler.cpp
    src/record/RecordManager.cpp
    src/record/RMComponent.cpp
)

set(HEADERS
	${HEADERS}
)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_library(${CMAKE_PROJECT_NAME}_lib ${SOURCE} ${HEADERS})
add_executable(${CMAKE_PROJECT_NAME} src/test.cpp)
# target_link_libraries(${CMAKE_PROJECT_NAME} sql_parser ${CMAKE_PROJECT_NAME}_lib)

#add_executable(test_table test/test_table.cpp)
#target_link_libraries(test_table sql_parser ${CMAKE_PROJECT_NAME}_lib)

