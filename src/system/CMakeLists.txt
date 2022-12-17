set(SOURCE
    ${SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/DatabaseManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TableManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/testSystem.cpp
    PARENT_SCOPE
)

set(HEADERS
    ${HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/DatabaseManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/TableManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DBComponent.h
    PARENT_SCOPE
)