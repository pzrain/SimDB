#ifndef __COMMON_H__
#define __COMMON_H__

// Define the constants used
// According to the FAQ

/* DataBase Info */
#define DB_MAX_TABLE_NUM 8
#define DB_MAX_NAME_LEN 32

/* Table Info */
#define TAB_MAX_COL_NUM 10
#define TAB_MAX_NAME_LEN 64
#define TAB_MAX_LEN 128

/* Parser */
#define MAX_INPUT_SIZE 150

typedef enum {
    COL_NULL = 0,
    COL_INT = 1,
    COL_VARCHAR = 2,
    COL_FLOAT = 3
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