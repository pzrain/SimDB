set(SOURCE
    ${SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/MyBitMap.cpp
    PARENT_SCOPE
)

set(HEADERS
    ${HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/bufmanager/BufPageManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/bufmanager/FindReplace.h
	${CMAKE_CURRENT_SOURCE_DIR}/fileio/FileManager.h
	${CMAKE_CURRENT_SOURCE_DIR}/fileio/FileTable.h
	${CMAKE_CURRENT_SOURCE_DIR}/utils/MyBitMap.h
	${CMAKE_CURRENT_SOURCE_DIR}/utils/MyHashMap.h
	${CMAKE_CURRENT_SOURCE_DIR}/utils/MyLinkList.h
	${CMAKE_CURRENT_SOURCE_DIR}/utils/pagedef.h
    PARENT_SCOPE
)