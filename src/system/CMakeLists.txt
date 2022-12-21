set(SOURCE
    ${SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/DatabaseManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TableManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
    PARENT_SCOPE
)

set(HEADERS
    ${HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/DatabaseManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/TableManager.h
    PARENT_SCOPE
)