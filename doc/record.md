## <center>记录管理</center>

按我的理解，记录管理模块是实际“**操作**”数据库中的数据的模块，由它来指定什么时候创建文件（创建数据库中的一张表）、删除文件，读写文件的内容（在表中查询、删除、修改数据）。它需要规定文件中存储数据的格式（例如每页中存放多少条数据记录，以及位图的格式），还需要存储表的一些元数据，这可以存放在对应文件的第一**页**，但是其不直接进行文件的IO，而是调用页式文件系统中的相关接口。

### RecordId

> `src/record/RMComponent.h`

用于定位一条记录，包括记录存在哪一页哪一槽，后续可以通过`RecordId`调用相关接口得到`Record`。

```C++
class RecordId{
private:
    int pageId, slotId;
public:
    RecordId(int pageId_, int slotId_) {
        pageId = pageId_;
        slotId = slotId_;
    }

    ~RecordId() {}

    int getpageId() {
        return pageId;
    }

    int getslotId() {
        return slotId;
    }

    void set(int pageId_, int slotId_) {
        pageId = pageId_;
        slotId = slotId_;
    }
};
```

### Record

> `src/record/RMComponent.h`

描述了存放在文件里的一条记录，也即插入数据库表中的一条记录，为序列化之后的结果。

```C++
struct Record{
    uint8_t* data;
};
```

在页式文件系统中，文件（也就是一张表）被划分为很多页。为了快速找到空闲的页上空闲的槽，空闲页面被组成成链表的形式，链表头部空闲页的页号被记录在表头中。同时，每个页面中的空闲槽也被组织成链表，页头中记录的指针指向链表开头的空闲槽。如果插入一条数据，就在页头中记录的空闲槽位中插入，同时指针后移一位；如果删除一条记录，就将删除后得到的空闲槽插入链表开头，并让页头中的指针指向它。

### PageHeader

`src/record/RMComponent.h`

页头。

```C++
struct PageHeader{
    int16_t nextFreePage;
    int16_t firstEmptySlot; // slot id starts at 0
    int16_t totalSlot;

    PageHeader() {
        nextFreePage = -1;
        firstEmptySlot = -1;
        totalSlot = 0;
    }
};
```

### TableHeader

`src/record/RMComponent.h`

用来记录表的列数、列属性等**元数据**。

```C++
class TableHeader{
private:
    void calcRecordSizeAndLen(); // 计算其中一条记录的长度，以及一页可以存放的记录数
public:
    uint8_t valid;
    uint8_t colNum;
    int16_t firstNotFullPage;
    uint16_t recordLen, totalPageNumber; // recordLen: length of one record
    uint32_t recordSize; // total number of records/slots

    TableEntry* entryHead;
    char tableName[TAB_MAX_NAME_LEN];

    TableHeader();

    ~TableHeader();

    TableEntry* getCol(char* colName);

    void init(TableEntry* entryHead_);

    int alterCol(TableEntry* tableEntry);

    int removeCol(char* colName);

    int addCol(TableEntry* tableEntry);

    bool existCol(char* colName);

    void printInfo();
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
public:

    FileHandler();

    ~FileHandler();

    void init(BufPageManager* bufPageManager_, int fileId_, const char* filename);

    int getFileId();

    int operateTable(TB_OP_TYPE opCode, char* colName = nullptr, TableEntry* tableEntry = nullptr);
    // when operating type is init, tableEntry represents the head of a linklist of TableEntry
    
    bool getRecord(RecordId recordId, Record &record);

    bool insertRecord(RecordId &recordId, const Record &record);

    bool removeRecord(const int recordId);

    bool updateRecord(const Record &record);

    bool writeBack(const int pageNumber); // write page back to disk
};
```

`FileHandler`通过接口`operateTable()`来对一张表的结构进行初始化、修改、增加删除等操作，`opCode`指定的操作的类型。返回值为0表示操作成功，-1表示操作失败。特别的，`opCode=TB_EXIST`时，返回值为0表示未找到对应列，返回值为1表示找到了。

### RecordManager

> `src/record/RecordManager.h`

用于管理所有打开的文件，或是对文件进行创建、删除等操作。可以理解为统一管理对于一个数据库下的多张表，而每一张表的具体处理由`FileHandler`来实行。

```C++
class RecordManager{
private:
    FileManager* fileManager;
public:
    RecordManager(FileManager*);

    ~RecordManager();

    void createFile(const char* filename); 

    void removeFile(const char* filename);

    void openFile(const char* filename, FileHandler* fileHandler);// one file corresponds with one fileHandler
    //根据数据库和表名确定文件路径，打开文件后将控制权传递给FileHandler

    void closeFile(FileHandler* fileHandler);
};

#endif
```

