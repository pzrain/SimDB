#include "RMComponent.h"
#include "../filesystem/utils/pagedef.h"
#include "cstring"

void loadTableHeaderToBuff(BufType &dataLoad, TableHeader* tableHeader) {
    memcpy((uint8_t*)tableHeader, (uint8_t*)dataLoad, sizeof(TableHeader));
    // tableHeader = (TableHeader*)((uint8_t*)dataLoad[sizeof(TableHeader)]);
}

void writeTableHeaderToBuff(BufType &dataLoad, TableHeader* tableHeader) {
    memcpy((uint8_t*)dataLoad, (uint8_t*)tableHeader, sizeof(TableHeader));
}

void TableEntry::calcRecordLen() {
    recordLen = 16 + TAB_MAX_NAME_LEN + sizeof(void*) + TAB_MAX_LEN;
}

TableHeader::TableHeader() {
    valid = 0;
    colNum = 0;
    firstNotFullPage = -1;
    totalPageNumber = 1;
    recordSize = 0;
    recordLen = 0;
    entryHead = nullptr;
}

TableHeader::~TableHeader() {
    TableEntry* head = entryHead, *next;
    while (head) {
        next = head->next;
        delete head;
        head = next;
    }
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
            recordSize = recordSize - head->recordLen + tableEntry->recordLen;
            delete head;
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
            recordSize -= head->recordLen;
            delete head;
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
    recordSize += tableEntry->recordLen;
    return 0;
}

void TableHeader::init(TableEntry* entryHead_) {
    entryHead = entryHead_;
    calcRecordSizeAndLen();
    valid = 1;
}

void TableHeader::calcRecordSizeAndLen() {
    TableEntry* head = entryHead;
    recordLen = sizeof(uint16_t); // record if slot is free
    while (head) {
        recordLen += head->recordLen;
        head = head->next;
    }
    if (recordSize > 0) {
        recordSize = (PAGE_SIZE - sizeof(PageHeader)) / recordLen;
    }
}