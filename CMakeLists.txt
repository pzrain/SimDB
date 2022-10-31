cmake_minimum_required (VERSION 2.6)

project(SimDB)

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()
# set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

set(SOURCE
	src/record/FileHandler.cpp
    src/record/RecordManager.cpp
    src/record/RMComponent.cpp
	src/filesystem/utils/MyBitMap.cpp
	src/main.cpp
)

set(HEADERS
	src/record/FileHandler.h
	src/record/RecordManager.h
	src/record/RMComponent.h
	src/filesystem/bufmanager/BufPageManager.h
	src/filesystem/bufmanager/FindReplace.h
	src/filesystem/fileio/FileManager.h
	src/filesystem/fileio/FileTable.h
	src/filesystem/utils/MyBitMap.h
	src/filesystem/utils/MyHashMap.h
	src/filesystem/utils/MyLinkList.h
	src/filesystem/utils/pagedef.h
	src/common.h
)
SET(GCC_COVERAGE_COMPILE_FLAGS "-g")

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})
ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${SOURCE} ${HEADERS})

