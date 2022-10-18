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
    COL_INT = 0,
    COL_VARCHAR = 1,
    COL_FLOAT = 2
} TB_COL_TYPE;

#endif