#ifndef __COMMON_H__
#define __COMMON_H__

// Define the constants used
// According to the FAQ

/* DataBase Info */
#define DB_MAX_TABLE_NUM 8

/* Table Info */
#define TAB_MAX_COL_NUM 10
#define TAB_MAX_NAME_LEN 64
#define TAB_MAX_LEN 128

typedef enum {
    COL_NULL = 0,
    COL_INT = 1,
    COL_VARCHAR = 2,
    COL_FLOAT = 3
} TB_COL_TYPE;

#endif