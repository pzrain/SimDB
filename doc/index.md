## <center>索引管理</center>

索引管理也是直接与最底层的页式文件系统进行交互，主要作用在于加速查询，以及处理主键约束、唯一约束、外键约束等。对某个数据库下某张表里的某一列，可以对其建立索引，也即为其创建一个B+树，将该索引的值作为key存储到B+树中，而该索引所对应记录的位置作为val。

查询时，指定key，会返回val，也就得到了对应key的记录存放的位置，从而起到了加速查询的作用。

处理主键约束时，指定key，如果返回的val的数量超过1，就证明数据库中存在两个相同的主键，也即不满足主键约束。

### IndexManager

`src/index/IndexManager.h`

在外部模块调用索引管理时，应当只需要调用`IndexManager`中的接口，其他的包括`BPlusTree`中的接口都已经在`IndexManager`中封装好。

注意在建立对应于某个数据库的`IndexManager`前，需要先建立好对应的数据库。具体接口的作用参见注释，而`IndexManager`的实际用法可以参见`src/index/testIndex.cpp`

具体的，需要先`createIndex`，然后调用`insert`将现有的索引全部插入。调用`search`或`searchBetween`进行查询。查询结束后可以`dropIndex`来删除之前建立的索引。

```C++

class IndexManager{
private:
    BufPageManager* bufPageManager;
    char databaseName[DB_MAX_NAME_LEN];
    char tableNames[DB_MAX_TABLE_NUM][TAB_MAX_NAME_LEN];
    char indexNames[DB_MAX_TABLE_NUM][TAB_MAX_COL_NUM][TAB_MAX_NAME_LEN];
    BPlusTree* bPlusTree[DB_MAX_TABLE_NUM][TAB_MAX_COL_NUM];
    int fileIds[DB_MAX_TABLE_NUM][TAB_MAX_COL_NUM];

    BPlusTree* findIndex(const char* tableName, const char* indexName);

    int findEmptyIndex(int &emptyI, int &emptyJ);
    
public:
    IndexManager(BufPageManager* bufPageManager_, char* databaseName_);

    ~IndexManager();

    int createIndex(const char* tableName, const char* indexName, uint16_t indexLen, uint8_t colType);
    // build index
    // return 0 if succeed, otherwise -1
    // so are the returning values of the following functions

    int removeIndex(const char* tableName, const char* indexName);
    // drop index, actually drop the corresponding B+ tree

    bool hasIndex(const char* tableName, const char* indexName);
    // return true if indexName has been created

    int insert(const char* tableName, const char* indexName, void* data, const int val);

    int insert(const char* tableName, const char* indexName, std::vector<void*> data, std::vector<int> val);
    // insert index in batch mode
    // the sequence of data is not necessarily ordered

    int search(const char* tableName, const char* indexName, void* data, std::vector<int> &res);
    // search those index whose key is equal to data
    // than store their val in vector res

    int searchBetween(const char* tableName, const char* indexName, void* lData, void* rData, std::vector<int> &res);
    // search those index whose dat falls in [lData, rData]
    // set lData or rData to nullptr to get searching range [lData, infinity] or [-infinity, rData]
    // example:
    // int lData = 3;
    // searchBetween(tableName, indexName, &lData, nullptr, res);

    int remove(const char* tableName, const char* indexName, void* data);
    // remove those index whose key is equal to data
};

#endif
```

### BPlusTree

`src/index/BPlusTree.h`

实现了一棵B+树，以及其相关的插入、删除、查找。对B+树的操作参考了实验文档上的说明。m阶B+树上所有非根页的内部节点数处于$[\lfloor(m+1)/2\rfloor,m]$

```C++
class BPlusTree{
private:
    int fileId;
    int rootIndex, tableIndex;
    uint16_t indexLen;
    uint8_t colType;
    BufPageManager* bufPageManager;
    IndexHeader* indexHeader;

    void writeIndexTable();

    int getNextFreePage(); // alloc one empty page

    inline void transform(int& val, int pageId, int slotId);

    inline void transformR(int val, int& pageId, int& slotId);

    int searchUpperBound(void* data);

    void recycle(std::vector<IndexPage*> &rec, std::vector<int> &pageIndex, bool writeback = false);

    void dealOverFlow(IndexPage* indexPage, std::vector<IndexPage*> &rec, std::vector<int> &pageIndex);

    void dealUnderFlow(IndexPage* indexPage, std::vector<IndexPage*> &rec, std::vector<int> &pageIndex);

    void update(IndexPage* indexPage, void* updateData, std::vector<IndexPage*> &rec, std::vector<int> &pageIndex);

public:
    IndexPage* root;

    BPlusTree(int fileId_, BufPageManager* bufPageManager_, uint16_t indexLen_, uint8_t colType_);

    ~BPlusTree();

    void search(void* data, std::vector<int> &res);
    // attention that res will be cleared first
    // return value of those slots whose data_ is equla to data

    void searchBetween(void* ldata, void* rdata, std::vector<int> &res);
    // bound [ldata, rdata]
    // set ldata or rdata to nullptr if want a one-way search

    void insert(void* data, const int val);

    void remove(void* data);
};
```

### IndexPage

`src/index/IndexPage.h`

维护了内存中的一个索引页。

内存页中的每个节点是按照键值从小到大进行排序的，对每个节点，维护了其上一个节点的位置、下一个节点的位置、存储的值（val）、孩子页的位置等。内存中某一文件内各页的组织方式与记录管理模块中类似。

```C++
typedef enum{
    INDEX_PAGE_INTERIOR = 0,
    INDEX_PAGE_LEAF = 1
} INDEX_PAGE_TYPE;

struct IndexPageHeader{
    uint8_t isInitialized;
    uint8_t colType;
    uint8_t pageType;
    int16_t nextPage; // related to B+ tree
    int16_t lastPage;
    int16_t firstIndex;
    int16_t lastIndex;
    int16_t firstEmptyIndex;
    int16_t nextFreePage;
    uint16_t totalIndex;
    uint16_t indexLen;
    int fatherIndex; // fatherIndex will be built from B+ tree

    void initialize(uint16_t indexLen_, uint8_t colType_);

    inline uint16_t getTotalLen();
};

class IndexPage{
private:
    IndexPageHeader* indexPageHeader;
    Compare* compare;
    uint16_t capacity;
    uint16_t pageId;

public:
    uint8_t* data;

    IndexPage(uint8_t* pageData, uint16_t indexLen, uint8_t colType, uint16_t pageId_);

    ~IndexPage();

    void initialize(uint16_t indexLen_, uint8_t colType);

    void clear();

    uint16_t getCapacity();

    void changePageType(uint8_t newPageType);

    Compare* getCompare();

    uint8_t getPageType();

    uint16_t getTotalIndex();

    uint16_t getPageId();

    bool overflow();

    bool underflow();

    uint8_t* accessData(int id); // get one item accordin to specified slotId

    void* getData(int id); // get true data

    int* getVal(int id);

    int16_t* getNextIndex(int id);

    int16_t* getLastIndex(int id); // return the last index of this slot

    int16_t* getLastIndex(); // return the last index on this page

    int16_t* getFirstIndex();

    int16_t* getChildIndex(int id);

    int16_t* getNextPage();

    int16_t* getLastPage();

    int16_t* getNextFreePage();

    int16_t* getFirstEmptyIndex();

    int* getFatherIndex();

    int cut(int k);

    int insert(void* data, const int val, const int16_t childIndex_ = -1); // key and value
                                                                           // return the slot index of the inserted value

    int insert(std::vector<void*> data, std::vector<int> val, std::vector<int16_t> childIndex, bool front);
    // return 0 if all the data can fit in this page
    // otherwise -1, and no data will be inserted
    // attention: data should be aranged in no descending order
    //            those data will be inserted right in the end of this page(front = false)
    //                                           or in the front of this page(front = true)

    void searchEQ(void* data, std::vector<int> &res);

    int searchLowerBound(void* data); // return the uppermost index of slots whose value is no greater than data
                                      // -1 if all value are greater than data
    
    int searchUpperBound(void* data); // similar to searchLowerBound()

    void remove(void* data, std::vector<int> &res, int cnt = -1);
    // remove data from page
    // cnt defines the amount of data you want to remove. to remove all, set cnt to -1

    void removeSlot(int slotId);
    // remove data of the specified slot, return 0 if succeed, -1 if fail
    // attention: this method will do extra test(like if the slot is empty or not)
    //            SO CAREFULLY CALL!

    void removeFrom(int16_t index, std::vector<void*>& removeData, std::vector<int>& removeVal, std::vector<int16_t>& removeChildIndex);
    // attention: void* in removeData will only be temporary valid, the value it point to may be overwrite!
    // this interface can only be used in spliting a page, afterwards writing part of its content to another page

    void insertFrom(const std::vector<void*> insertData, const std::vector<int> val, const std::vector<int16_t> insertChildIndex);
    // attention: insertData and insertVal should come directly from removeData and removeVal in removeFrom, respectively
    // this interface can only be used in spliting a page, afterwards writing part of its content to another page
};
```

