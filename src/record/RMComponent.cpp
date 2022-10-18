#include "RMComponent.h"
#include "../filesystem/utils/pagedef.h"
#include "cstring"

void loadTableHeaderToBuff(BufType dataLoad, TableHeader* tableHeader) {
    memcpy((uint8_t*)tableHeader, (uint8_t*)dataLoad, sizeof(TableHeader));
    // tableHeader = (TableHeader*)((uint8_t*)dataLoad[sizeof(TableHeader)]);
}

void writeTableHeaderToBuff(BufType dataLoad, TableHeader* tableHeader) {
    memcpy((uint8_t*)dataLoad, (uint8_t*)tableHeader, sizeof(TableHeader));
}

TableHeader::TableHeader() {
    valid = 0;
    colNum = 0;
    recordSize = 0;
    entryHead = nullptr;
}

bool TableHeader::existCol(char* colName) {
    TableEntry* head = entryHead;
    while (head) {
        if (strcmp(colName, head->colName) == 0) {
            return true;
        }
        head = head->next;
    }
    return false;
}

void TableHeader::printInfo() {
    printf("==========  Begin Table Info  ==========\n");

    printf("Table name: %s", tableName);
    printf("Column size: %d", colNum);
    printf("Record size: ", recordSize);

    TableEntry* head = entryHead;
    while (head) {
        printf("[column %d] name = %s, type = ", head->colName);
        if (head->uniqueConstraint) {
            printf("Unique ");
        }
        if (head->primaryKeyConstraint) {
            printf("Primary Key ");
        }
        switch (head->colType) {
            case COL_INT:
                printf("INT");
                if (head->hasDefault) {
                    printf(", Default value = %d", head->defaultValInt);
                }
                break;
            case COL_VARCHAR:
                printf("VARCHAR(%d)", head->colLen);
                if (head->hasDefault) {
                    printf(", Default value = %s", head->defaultValVarchar);
                }
                break;
            case COL_FLOAT:
                printf("FLOAT");
                if (head->hasDefault) {
                    printf(", Default value = %f", head->defaultValFloat);
                }
                break;
            case COL_NULL:
                printf("NULL");
                break;
            default:
                printf("Error Type");
                break;
        }
        if (head->notNullConstraint) {
            printf(", not NULL");
        }
        printf("\n");
        head = head->next;
    }

    printf("==========   End  Table Info  ==========\n");
}

TableEntry* TableHeader::getCol(char* colName) {
    TableEntry* head = entryHead;
    while (head) {
        if (strcmp(head->colName, colName) == 0) {
            return head;
        }
        head = head->next;
    }
    return nullptr;
}

int TableHeader::alterCol(TableEntry* tableEntry) {
    TableEntry* head = entryHead;
    TableEntry* prev = nullptr;
    while (head) {
        if (strcmp(head->colName, tableEntry->colName) == 0) {
            if (prev) {
                prev->next = tableEntry;
            } else {
                entryHead = tableEntry;
            }
            tableEntry->next = head->next;
            return 0;
        }
        prev = head;
        head = head->next;
    }
    return -1;
}

int TableHeader::removeCol(char* colName) {
    TableEntry* head = entryHead;
    TableEntry* prev = nullptr;
    while (head) {
        if (strcmp(head->colName, colName) == 0) {
            if (prev) {
                prev->next = head->next;
            } else {
                entryHead = head->next;
            }
            return 0;
        }
    }
    return -1;
}

int TableHeader::addCol(TableEntry* tableEntry) {
    if (existCol(tableEntry->colName)) {
        printf("[Error] Column with same name already exists.\n");
        return -1;
    }
    TableEntry* head = entryHead;
    if (!head) {
        head = tableEntry;
    } else {
        while (head->next)
            head = head->next;
        head->next = entryHead;
    }
    return 0;
}

void TableHeader::init(TableEntry* entryHead_) {
    entryHead = entryHead_;
    valid = 1;
}
