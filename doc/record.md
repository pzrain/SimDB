## <center>记录管理</center>

按我的理解，记录管理模块是实际“**操作**”数据库中的数据的模块，由它来指定什么时候创建文件（创建数据库中的一张表）、删除文件，读写文件的内容（在表中查询、删除、修改数据）。它需要规定文件中存储数据的格式（例如每页中存放多少条数据记录，以及位图的格式），还需要存储表的一些元数据，这可以存放在对应文件的第一**页**，但是其不直接进行文件的IO，而是调用页式文件系统中的相关接口。

### Record

> `src/record/RMComponent.h`

描述了存放在文件里的一条记录，也即插入数据库表中的一条记录。

```C++
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
```

### TableHeader

`src/record/RMComponent.h`

用来记录表的列数、列属性等**元数据**。

```C++
class TableHeader{
public:
    uint8_t valid;
    uint8_t colNum;
    uint32_t recordSize;
    TableEntry* entryHead;
    char tableName[TAB_MAX_NAME_LEN];

    TableHeader();

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
    FileManager* fileManager;
    TableHeader* tableHeader;
    int fileId;
public:

    FileHandler();

    ~FileHandler();

    void init(FileManager* fileManager_, int fileId_);

    int getFileId();

    int operateTable(TB_OP_TYPE opCode, char* colName = nullptr, TableEntry* tableEntry = nullptr);
    // when operating type is init, tableEntry represents the head of a linklist of TableEntry
    
    bool getRecord(const int recordId, Record &record) const;

    bool insertRecord(const char* recordData, int &recordId);

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

