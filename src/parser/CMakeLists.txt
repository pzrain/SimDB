FILE(GLOB parser_source "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB parser_antlr_source "${CMAKE_CURRENT_SOURCE_DIR}/antlr/*.cpp")
FILE(GLOB parser_antlr_src_source "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/*.cpp")
FILE(GLOB parser_antlr_src_atn_source "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/atn/*.cpp")
FILE(GLOB parser_antlr_src_dfa_source "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/dfa/*.cpp")
FILE(GLOB parser_antlr_src_internal_source "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/internal/*.cpp")
FILE(GLOB parser_antlr_src_misc_source "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/misc/*.cpp")
FILE(GLOB parser_antlr_src_support_source "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/support/*.cpp")
FILE(GLOB parser_antlr_src_tree_source "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/tree/*.cpp")
FILE(GLOB parser_antlr_src_tree_pattern_source "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/tree/pattern/*.cpp")
FILE(GLOB parser_antlr_src_tree_xpath_source "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/tree/xpath/*.cpp")

FILE(GLOB parser_header "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
FILE(GLOB parser_antlr_header "${CMAKE_CURRENT_SOURCE_DIR}/antlr/*.h")
FILE(GLOB parser_antlr_src_header "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/*.h")
FILE(GLOB parser_antlr_src_atn_header "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/atn/*.h")
FILE(GLOB parser_antlr_src_dfa_header "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/dfa/*.h")
FILE(GLOB parser_antlr_src_internal_header "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/internal/*.h")
FILE(GLOB parser_antlr_src_misc_header "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/misc/*.h")
FILE(GLOB parser_antlr_src_support_header "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/support/*.h")
FILE(GLOB parser_antlr_src_tree_header "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/tree/*.h")
FILE(GLOB parser_antlr_src_tree_pattern_header "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/tree/pattern/*.h")
FILE(GLOB parser_antlr_src_tree_xpath_header "${CMAKE_CURRENT_SOURCE_DIR}/antlr/src/tree/xpath/*.h")

set(HEADERS
    ${HEADERS}
	${parser_header}
    ${parser_antlr_header}
    ${parser_antlr_src_header}
    ${parser_antlr_src_atn_header}
    ${parser_antlr_src_dfa_header}
    ${parser_antlr_src_internal_header}
    ${parser_antlr_src_misc_header}
    ${parser_antlr_src_support_header}
    ${parser_antlr_src_tree_header}
    ${parser_antlr_src_tree_pattern_header}
    ${parser_antlr_src_tree_xpath_header}
    PARENT_SCOPE
)

set(SOURCE
    ${SOURCE}
    ${parser_source}
    ${parser_antlr_source}
    ${parser_antlr_src_source}
    ${parser_antlr_src_atn_source}
    ${parser_antlr_src_dfa_source}
    ${parser_antlr_src_internal_source}
    ${parser_antlr_src_misc_source}
    ${parser_antlr_src_support_source}
    ${parser_antlr_src_tree_source}
    ${parser_antlr_src_tree_pattern_source}
    ${parser_antlr_src_tree_xpath_source}
    PARENT_SCOPE
)
# FILE(GLOB parser_source "${CMAKE_CURRENT_SOURCE_DIR}/antlr/*.cpp")
# FILE(GLOB parser_header "${CMAKE_CURRENT_SOURCE_DIR}/antlr/*.h")

# set(SOURCE
#     ${SOURCE}
#     ${CMAKE_CURRENT_SOURCE_DIR}/MYParser.cpp
#     ${parser_source}
#     PARENT_SCOPE
# )

# set(HEADERS
#     ${HEADERS}
#     ${parser_header}
# 	${CMAKE_CURRENT_SOURCE_DIR}/ASTNode.h
# 	${CMAKE_CURRENT_SOURCE_DIR}/ASTBase.h
#     ${CMAKE_CURRENT_SOURCE_DIR}/MySQLVisitor.h
#     PARENT_SCOPE
# )