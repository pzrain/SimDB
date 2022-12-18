#ifndef __COMMON_H__
#define __COMMON_H__

// Define the constants used
// According to the FAQ

/* DataBase Info */
#define DB_MAX_TABLE_NUM 8
#define DB_MAX_NAME_LEN 32

/* Table Info */
#define COL_MAX_NAME_LEN 64
#define TAB_MAX_COL_NUM 10
#define TAB_MAX_NAME_LEN 64
#define TAB_MAX_LEN 128

/* System */
#define MAX_FOREIGN_KEY_NUM 20
#define MAX_FOREIGN_KEY_FOR_COL 5

typedef enum {
    ORD_TYPE = 0,   // no aggregation operator, like MAX, MIN, SUM, COUNT, AVG
    MAX_TYPE = 1,
    MIN_TYPE = 2,
    SUM_TYPE = 3,
    COUNT_TYPE = 4,
    AVERAGE_TYPE = 5
} DB_SELECT_TYPE;

typedef enum {
    EQU_TYPE = 0,  // =
    NEQ_TYPE = 1,  // !=
    GT_TYPE  = 2,  // >
    GTE_TYPE = 3,  // >=
    LT_TYPE  = 4,  // <
    LTE_TYPE = 5,  // <=
    IS_TYPE  = 6,  // IS
    ISN_TYPE = 7,  // IS NOT
    IN_TYPE  = 8,  // IN
    LIKE_TYPE= 9   // LIKE
} DB_EXP_OP_TYPE;

typedef enum {
    DB_NULL = 0,
    DB_INT = 1,
    DB_FLOAT = 2,
    DB_CHAR = 3,
    DB_LIST = 4,         // value_list
    DB_ITEM = 5,         // tableName.colName or colName
    DB_NST  = 6          // nest selection
} DB_EXP_TYPE;

typedef enum {
    DB_LIST_NULL = 0,
    DB_LIST_INT = 1,
    DB_LIST_FLOAT = 2,
    DB_LIST_CHAR = 3
} DB_LIST_TYPE;

/* End System */

/* Parser */
#define MAX_INPUT_SIZE 150
/* End Parser */

typedef enum {
    COL_NULL = 0,
    COL_INT = 1,
    COL_FLOAT = 2,
    COL_VARCHAR = 3
} TB_COL_TYPE;

typedef enum {
    EQUAL = 0,
    LESS = 1,
    LESS_EQUAL = 2,
    GREATER = 3,
    GREATER_EQUAL = 4,
    NOT_EQUAL = 5
} CHECK_TYPE;
#endif