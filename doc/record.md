## <center>记录管理</center>

按我的理解，记录管理模块是实际“**操作**”数据库中的数据的模块，由它来指定什么时候创建文件（创建数据库中的一张表）、删除文件，读写文件的内容（在表中查询、删除、修改数据）。它需要规定文件中存储数据的格式（例如每页中存放多少条数据记录，以及位图的格式），还需要存储表的一些元数据，这可以存放在对应文件的第一**页**，但是其不直接进行文件的IO，而是调用页式文件系统中的相关接口。

### RecordId

> `src/record/RMComponent.h`

用于定位一条记录，包括记录存在哪一页哪一槽，后续可以通过`RecordId`调用相关接口得到`Record`。

```C++
class RecordId{
private:
    int16_t pageId, slotId;
public:
    RecordId() {}

    RecordId(int16_t pageId_, int16_t slotId_) {
        pageId = pageId_;
        slotId = slotId_;
    }

    ~RecordId() {}

    int16_t getPageId() {
        return pageId;
    }

    int16_t getSlotId() {
        return slotId;
    }

    void set(int16_t pageId_, int16_t slotId_) {
        pageId = pageId_;
        slotId = slotId_;
    }
};
```

### Record

> `src/record/RMComponent.h`

描述了存放在文件里的一条记录，也即插入数据库表中的一条记录，为序列化之后的结果。`bitmap`是记录`record`中的每一项是否为`NULL`的位图。由于最多有10列，因此`bitmap`占用两字节的空间即可保证够用。**创建时应当指定记录的长度**，可以调用`FileHandler->getRecordLen()`得到。或者，通过`TableHeader->recordLen`得到，但是注意这里的`recordLen`应当减去2（即用于记录slot是否空闲的域的大小）。

```C++
class Record{
public:
    uint8_t* data; // The first two byte of data is set to be bitmap
    size_t len;
    uint16_t* bitmap;

    Record(size_t len_) {
        len = len_;
        data = new uint8_t[len_];
        bitmap = (uint16_t*)data;
        *bitmap = 0;
    }

    void setItemNull(int index) {
        assert(index >= 0 && index < 16);
        *bitmap = (*bitmap) | (1 << index);
    }

    void setItemNotNull(int index) {
        assert(index >= 0 && index < 16);
        if ((*bitmap >> index) & 1) {
            *bitmap = (*bitmap) ^ (1 << index);
        }
    }

    bool isNull(int index) {
        assert(index >= 0 && index < 16);
        return (((*bitmap) >> index) & 1);
    }

    ~Record() {
        delete data;
    }

    bool deserialize(RecordData& rData, TableEntryDesc& tableEntryDesc);
};
```

在页式文件系统中，文件（也就是一张表）被划分为很多页。为了快速找到空闲的页上空闲的槽，空闲页面被组成成链表的形式，链表头部空闲页的页号被记录在表头中。同时，每个页面中的空闲槽也被组织成链表，页头中记录的指针指向链表开头的空闲槽。如果插入一条数据，就在页头中记录的空闲槽位中插入，同时指针后移一位；如果删除一条记录，就将删除后得到的空闲槽插入链表开头，并让页头中的指针指向它。

### Serialize & Deserialize

`src/record/RMComponent.h`

用于序列化&反序列化的相关操作。`RecordData`是未进行序列化的原始信息。通过`RecordData.serialize()`进行序列化后存储在`Record`中。`TableEntryDesc`是对表中各列的描述信息，用于指导反序列化操作。通过`Record.deserialize(RecordData, TableEntryDesc)`来将`Record`中储存的序列化后的字节序列再反序列化，存储到`RecordData`中。需要注意的是，`RecordData`实际上是一个节点的列表，每个节点是一个指向某一列具体类型、长度信息的结构体的指针。这些指针的内存管理在`RecordData`类中已经做好，不可以手动`delete`。

```C++
struct RecordDataNode{
    union{
        int* intContent;
        float* floatContent;
        char* charContent;
    }content;
    uint32_t len;
    TB_COL_TYPE nodeType;
    RecordDataNode* next = nullptr;

    ~RecordDataNode() {}
};

class RecordData{
public:
    RecordDataNode* head; // attention : you should not delete head manually
    size_t recordLen = 0;

    RecordData() { head = nullptr; }

    RecordData(RecordDataNode* head_): head(head_) {}
    
    bool serialize(Record& record);

    size_t getLen();

    ~RecordData();
};

struct TableEntryDescNode{
    uint8_t colType;
    uint32_t colLen;
    TableEntryDescNode* next;
};

class TableEntryDesc{
private:
    size_t len = 0;
public:
    TableEntryDescNode* head;

    ~TableEntryDesc();

    size_t getLen();
};
```

### PageHeader

`src/record/RMComponent.h`

页头，记录某一页的一些基本信息

```C++
struct PageHeader{
    int16_t nextFreePage;
    int16_t firstEmptySlot; // slot id starts at 0
    int16_t totalSlot;
    int16_t maximumSlot;

    PageHeader() {
        nextFreePage = -1;
        firstEmptySlot = -1;
        totalSlot = 0;
        maximumSlot = -1;
    }
};
```

### TableEntry

`src/record/RMComponent.h`

记录的是表中某一列的基本信息，包括类型（`INT`，`FLOAT`，`VARCHAR`），约束等。需要注意的是，`TableEntry.next`是与所在表相关的，用来构建链表。

还需要实现的是，设计记录`checkConstraint`以及`foreignKeyConstraint`的域，然后在接口`verifyConstraint`中对其进行判断。实际进行约束检查时，会调用`TableHeader->verifyConstraint()`来对整条记录进行判断，首先会将记录进行反序列化，然后依次遍历反序列化得到的记录中各项连成的列表，调用其对应的`TableEntry`的`verifyConstraint()`方法来检查。

```C++
struct TableEntry{
    uint8_t colType;
    bool checkConstraint;
    bool primaryKeyConstraint;
    bool foreignKeyConstraint;
    uint32_t colLen; // VARCHAR(%d), int(4), float(4)
    char colName[TAB_MAX_NAME_LEN];
    bool hasDefault;
    bool notNullConstraint;
    bool uniqueConstraint;
    bool isNull;
    union {
        int defaultValInt;
        char defaultValVarchar[TAB_MAX_LEN];
        float defaultValFloat;
    };
    int8_t next; // don't forget to build the link 
    /* 
        TODO: implement of checkConstraint and foreignKeyConstraint
     */

    TableEntry();

    bool verifyConstraint(RecordDataNode* data); // TODO
    // verify checkConstraint, primaryKeyConstraint, notNullConstraint, UniqueConstraint
    // on deserialized data
    // return true if succeed else false
};
```

### TableHeader

`src/record/RMComponent.h`

用来记录表的列数、列属性等**元数据**。

```C++
class TableHeader{
private:
    void calcRecordSizeAndLen();

    int findFreeCol();

public:
    uint8_t valid;
    uint8_t colNum;
    int8_t entryHead;
    int16_t firstNotFullPage;
    uint16_t recordLen, totalPageNumber; // recordLen: length of one record
    uint32_t recordSize; // number of records/slots on one page
    uint32_t recordNum; // total number of records;
    TableEntry entrys[TAB_MAX_COL_NUM];
    char tableName[TAB_MAX_NAME_LEN];

    TableHeader();

    void init(TableEntry* entryHead_, int num);
    // num is the length of the TableEntry array
    // Attention: init will not check same column name, thus should only be called to initial a tableHeader

    int getCol(char* colName, TableEntry& tableEntry);

    int alterCol(TableEntry* tableEntry);
    // update a column
    // according to the doc, this method may not be called

    int removeCol(char* colName);

    int addCol(TableEntry* tableEntry);

    bool existCol(char* colName);

    void printInfo();
    
    TableEntryDesc getTableEntryDesc();
    
    bool verifyConstraint(const RecordData& recordData);

    bool verifyConstraint(Record& record);
};
```

### FileHandler

> `src/record/FileHandler.h`

用于维护打开的一个文件，这里的一个文件就是数据库中的一张表。

```C++
class FileHandler{
private:
    BufPageManager* bufPageManager;
    TableHeader* tableHeader;
    int fileId;
    char tableName[TAB_MAX_NAME_LEN];

public:

    FileHandler();

    ~FileHandler();

    void init(BufPageManager* bufPageManager_, int fileId_, const char* tableName_);

    int getFileId();

    char* getTableName();

    int operateTable(TB_OP_TYPE opCode, char* colName = nullptr, TableEntry* tableEntry = nullptr, int num = 0);
    /* 
        When operating type is TB_INIT, tableEntry represents an array of TableEntry

        TB_REMOVE, TB_EXIST requires parameter colName
        TB_INIT, TB_ALTER, TB_ADD requires parameter tableEntry
        TB_INIT requires parameter num, which is the total number of tableEntry array

        @return: -1 if fail, 
                 0 if succeed for all op other than TB_EXIST 
                 TB_EXIST returns 1 if exists otherwise 0
     */
    
    bool getRecord(RecordId recordId, Record &record);

    bool insertRecord(RecordId &recordId, Record &record);
    // the page id and slot id of the inserted record will be stored in recordId
    // at present, no constraints will be checked. (TODO)

    bool removeRecord(RecordId &recordId);

    bool updateRecord(RecordId &recordId, Record &record);
    // parameter recordId specifies the position of the record that needed to be updated
    // parameter record will substitutes the old record

    void getAllRecords(std::vector<Record*>&);
    // returns all records stores in this file

    bool insertAllRecords(const std::vector<Record*>&);
    // insert records in bulk

    int getRecordNum();

    size_t getRecordLen();

    TableEntryDesc getTableEntryDesc();

};
```

`FileHandler`通过接口`operateTable()`来对一张表的结构进行初始化、修改、增加删除等操作，`opCode`指定的操作的类型。返回值为0表示操作成功，-1表示操作失败。特别的，`opCode=TB_EXIST`时，返回值为0表示未找到对应列，返回值为1表示找到了。

### RecordManager

> `src/record/RecordManager.h`

用于管理所有打开的文件，或是对文件进行创建、删除等操作。可以理解为统一管理对于一个数据库下的多张表，而每一张表的具体处理由`FileHandler`来实行。

建立好`RecordManager`后，可以调用`isValid()`来判断其是否有效。（例如，如果相应的数据库还没有建立，就试图对其建立`RecordManager`，那么`isValid()`就会返回`false`）

```C++
class RecordManager{
private:
    BufPageManager* bufPageManager;
    char databaseName[DB_MAX_NAME_LEN];
public:
    RecordManager(BufPageManager*, char* databaseName_);

    ~RecordManager();

    int createFile(const char* tableName); 

    int removeFile(const char* tableName);

    int openFile(const char* tableName, FileHandler* fileHandler);// one file corresponds with one fileHandler

    int closeFile(FileHandler* fileHandler);
   
    bool isValid();
};
```

数据库的所有表文件存放在`database`文件夹下，命名为`{databaseName}/{tableName}.db`，以保证不会产生冲突。
