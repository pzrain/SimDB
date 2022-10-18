#ifndef __RMCOMPONENT_H_
#define __RMCOMPONENT_H_
#include "../common.h"
#include <cstdio>
#include <stdint.h>

class Record{
private:
    int pageNumber, slotNumber;
public:
    Record(int pageNumber_, int slotNumber_) {
        pageNumber = pageNumber_;
        slotNumber = slotNumber_;
    }

    ~Record() {}

    int getPageNumber() {
        return pageNumber;
    }

    int getSlotNumber() {
        return slotNumber;
    }
};

class TableHeader{
public:
    uint8_t valid;
    uint8_t colNum;
    uint8_t colType[TAB_MAX_COL_NUM];
    uint32_t colLen[TAB_MAX_COL_NUM]; // varchar(%d)
    uint32_t recordSize;
    char colName[TAB_MAX_COL_NUM][TAB_MAX_NAME_LEN];
    char tableName[TAB_MAX_NAME_LEN];
    // TODO: add more fields(including constraints)

    TableHeader() {
        valid = 0;
    }

    void printInfo() {
        printf("==========  Begin Table Info  ==========\n");

        printf("Table name: %s", tableName);
        printf("Column size: %d", colNum);
        printf("Record size: ", recordSize);

        for (int i = 0; i < colNum; ++i) {
            printf("[column %d] name = %s, type = ", i);
            switch (colType[i])
            {
            case COL_INT:
                printf("INT\n");
                break;
            case COL_VARCHAR:
                printf("VARCHAR(%d)\n", colLen[i]);
                break;
            case COL_FLOAT:
                printf("FLOAT\n");
                break;
            default:
                printf("Error Type\n");
                break;
            }
        }

        printf("==========   End  Table Info  ==========\n");
    }
};

#endif