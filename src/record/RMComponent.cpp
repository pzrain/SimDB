#include "RMComponent.h"
#include "../filesystem/utils/pagedef.h"
#include "cstring"

TableEntry::TableEntry() {}

TableEntry::TableEntry(char* colName_, uint8_t colType_, bool checkConstraint_, bool primaryKeyConstraint_, \
               bool foreignKeyConstraint_, uint32_t colLen_, bool hasDefault_, \
               bool notNullConstraint_, bool uniqueConstraint_, bool isNull_) {
    colType = colType_;
    strcpy(colName, colName_);
    // strncpy(colName, colName_, strlen(colName_));
    checkConstraint = checkConstraint_;
    primaryKeyConstraint = primaryKeyConstraint_;
    foreignKeyConstraint = foreignKeyConstraint_;
    colLen = colLen_;
    hasDefault = hasDefault_;
    notNullConstraint = notNullConstraint_;
    uniqueConstraint = uniqueConstraint_;
    isNull = isNull_;
    switch (colType) {
        case COL_INT:
            colLen = sizeof(int);
            break;
        case COL_FLOAT:
            colLen = sizeof(float);
            break;
        default:
            break;
    }
    next = -1;
}

TableHeader::TableHeader() {
    valid = 0;
    colNum = 0;
    entryHead = -1;
    firstNotFullPage = -1;
    totalPageNumber = 1;
    recordSize = 0;
    recordLen = 0;
    recordNum = 0;
}

bool TableHeader::existCol(char* colName) {
    int8_t head = entryHead;
    TableEntry entry;
    while (head >= 0) {
        entry = entrys[head];
        if (colName == nullptr) {
            printf("Error1\n");
            return false;
        }
        if (entry.colName == nullptr) {
            printf("Error2\n");
            return false;
        }
        if (strcmp(colName, entry.colName) == 0) {
            return true;
        }
        head = entry.next;
    }
    return false;
}

void TableHeader::printInfo() {
    printf("==========  Begin Table Info  ==========\n");

    printf("Table name: %s\n", tableName);
    printf("Column size: %d\n", colNum);
    printf("Record Length: %d\n", recordLen);
    printf("Record size: %d\n", recordSize);

    int8_t head = entryHead;
    TableEntry entry;
    int cnt = 0;
    while (head >= 0) {
        entry = entrys[head];
        printf("[column %d] name = %s, type = ", ++cnt, entry.colName);
        if (entry.uniqueConstraint) {
            printf("Unique ");
        }
        if (entry.primaryKeyConstraint) {
            printf("Primary Key ");
        }
        switch (entry.colType) {
            case COL_INT:
                printf("INT");
                if (entry.hasDefault) {
                    printf(", Default value = %d", entry.defaultValInt);
                }
                break;
            case COL_VARCHAR:
                printf("VARCHAR(%d)", entry.colLen);
                if (entry.hasDefault) {
                    printf(", Default value = %s", entry.defaultValVarchar);
                }
                break;
            case COL_FLOAT:
                printf("FLOAT");
                if (entry.hasDefault) {
                    printf(", Default value = %f", entry.defaultValFloat);
                }
                break;
            case COL_NULL:
                printf("NULL");
                break;
            default:
                printf("Error Type");
                break;
        }
        if (entry.notNullConstraint) {
            printf(", not NULL");
        }
        printf("\n");
        head = entry.next;
    }

    printf("==========   End  Table Info  ==========\n");
}

int TableHeader::getCol(char* colName, TableEntry& tableEntry) {
    int8_t head = entryHead;
    TableEntry entry;
    while (head >= 0) {
        entry = entrys[head];
        if (strcmp(entry.colName, colName) == 0) {
            tableEntry = entrys[head];
            return 0;
        }
        head = entry.next;
    }
    return -1;
}

int TableHeader::alterCol(TableEntry* tableEntry) {
    int8_t head = entryHead;
    TableEntry entry;
    while (head >= 0) {
        entry = entrys[head];
        if (strcmp(entry.colName, tableEntry->colName) == 0) {
            int8_t tmpNext = entry.next;
            memcpy(&(entrys[head]), tableEntry, sizeof(TableEntry));
            entrys[head].next = tmpNext;
            calcRecordSizeAndLen();
            return 0;
        }
        head = entry.next;
    }
    printf("[ERROR] column with specified name dose not exist.\n");
    return -1;
}

int TableHeader::removeCol(char* colName) {
    int8_t head = entryHead, prev = -1;
    TableEntry entry;
    while (head >= 0) {
        entry = entrys[head];
        if (strcmp(entry.colName, colName) == 0) {
            if (prev >= 0) {
                entrys[prev].next = entrys[head].next;
            } else {
                entryHead = entrys[head].next;
            }
            calcRecordSizeAndLen();
            colNum--;
            return 0;
        }
        prev = head;
        head = entry.next;
    }
    printf("[ERROR] column with specified name dose not exist.\n");
    return -1;
}

int TableHeader::findFreeCol() {
    bool check[TAB_MAX_COL_NUM] = {false};
    int8_t head = entryHead;
    while (head >= 0) {
        check[head] = true;
        head = entrys[head].next;
    }
    for (int i = 0; i < TAB_MAX_COL_NUM; i++) {
        if (!check[i])
            return i;
    }
    return -1;
}

int TableHeader::addCol(TableEntry* tableEntry) {
    if (existCol(tableEntry->colName)) {
        printf("[Error] Column with same name already exists.\n");
        return -1;
    }
    int8_t head = entryHead;
    TableEntry entry;

    int freeCol = findFreeCol();
    if (freeCol < 0) {
        printf("[ERROR] no free column.\n");
        return -1;
    }
    memcpy(&entrys[freeCol], tableEntry, sizeof(TableEntry));
    entrys[freeCol].next = -1;
    if (head < 0) {
        entryHead = freeCol;
    } else {
        entry = entrys[head];
        while (entry.next >= 0) {
            head = entry.next;
            entry = entrys[head];
        }
        entrys[head].next = freeCol;
    }
    colNum++;
    calcRecordSizeAndLen();
    return 0;
}

void TableHeader::init(TableEntry* entryHead_, int num) {
    entryHead = 0;
    for (int i = 0; i < num; i++) {
        entrys[i] = entryHead_[i];
        entrys[i].next = (i + 1) == num ? -1 : (i + 1);
    }
    colNum = num;
    calcRecordSizeAndLen();
    valid = 1;
}

void TableHeader::calcRecordSizeAndLen() {
    int8_t head = entryHead;
    TableEntry entry;
    recordLen = sizeof(uint16_t); // record if slot is free
    while (head >= 0) {
        entry = entrys[head];
        recordLen += entry.colLen;
        head = entry.next;
    }
    if (recordLen > 0) {
        recordSize = (PAGE_SIZE - sizeof(PageHeader)) / recordLen;
    }
}

TableEntryDesc::~TableEntryDesc() {
    TableEntryDescNode* cur = head;
    while (cur) {
        TableEntryDescNode* next = cur->next;
        delete cur;
        cur = next;
    }
}

TableEntryDesc TableHeader::getTableEntryDesc() {
    TableEntryDesc res;
    TableEntryDescNode* cur;
    TableEntryDescNode* prev = nullptr;
    int8_t head = entryHead;
    TableEntry entry;
    while (head >= 0) {
        entry = entrys[head];
        cur = new TableEntryDescNode();
        cur->colType = entry.colType;
        cur->colLen = entry.colLen;
        cur->next = nullptr;
        if (prev) {
            prev->next = cur;
        } else {
            res.head = cur;
        }
        prev = cur;
    }
    return res;
}

bool Record::deserialize(RecordData& rData, const TableEntryDesc& tableEntryDesc) {
    RecordDataNode* cur;
    RecordDataNode* prev = nullptr;
    TableEntryDescNode* now = tableEntryDesc.head;
    uint8_t* buf = data;
    size_t offset;
    while (now) {
        cur = new RecordDataNode();
        switch(now->colType) {
            case COL_INT:
                offset = sizeof(int);
                cur->content.intContent = new int;
                memcpy(cur->content.intContent, buf, offset);
                break;
            case COL_FLOAT:
                offset = sizeof(float);
                cur->content.floatContent = new float;
                memcpy(cur->content.floatContent, buf, offset);
                break;
            case COL_VARCHAR:
                offset = now->colLen;
                cur->content.charContent = new char;
                memcpy(cur->content.charContent, buf, offset);
                break;
            default:
                return false;
                break;
        }
        buf += offset;
        now = now->next;
    }
    printf("length = %d %d\n", (buf - data), strlen((char*)data));
    return (buf - data) == strlen((char*)data);
}

RecordDataNode::~RecordDataNode() {
    switch (nodeType) {
        case COL_INT:
            if (content.intContent)
                delete content.intContent;
            break;
        case COL_FLOAT:
            if (content.floatContent)
                delete content.floatContent;
                break;
        case COL_VARCHAR:
            if (content.charContent)
                delete content.charContent;
                break;
        default:
            break;
    }
}

RecordData::~RecordData() {
    RecordDataNode* cur = head;
    while (cur) {
        RecordDataNode* next = cur->next;
        delete cur;
        cur = next;
    }
}

bool RecordData::serialize(Record& record) {
    RecordDataNode* cur = head;
    uint8_t* buf = record.data;
    size_t offset;
    while (cur) {
        switch (cur->nodeType) {
            case COL_INT:
                offset = sizeof(int);
                memcpy(buf, cur->content.intContent, offset);
                break;
            case COL_FLOAT:
                offset = sizeof(float);
                memcpy(buf, cur->content.floatContent, offset);
                break;
            case COL_VARCHAR:
                offset = cur->len;
                memcpy(buf, cur->content.charContent, offset);
                break;
            default:
                return false;
                break;
        }
        buf += offset;
    }
    return true;
}
