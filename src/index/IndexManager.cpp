#include "IndexManager.h"
#include "unistd.h"
#include <sys/types.h>
#include <sys/stat.h>

IndexManager::IndexManager(BufPageManager* bufPageManager_, const char* databaseName_) {
    valid = true;
    bufPageManager = bufPageManager_;
    struct stat info;
    char databaseDirectory[DB_MAX_NAME_LEN + 30];
    sprintf(databaseDirectory, "database/%s", databaseName_);
    if(stat(databaseDirectory, &info ) != 0) {
        valid = false;
        printf("[ERROR] There is no directory %s.\n", databaseDirectory);
    }
    else if(!(info.st_mode & S_IFDIR)) {
        valid = false;
        printf("[ERROR] %s is not a directory.\n", databaseDirectory);
    }
    strcpy(databaseName, databaseName_);
    for (int i = 0; i < DB_MAX_TABLE_NUM; i++) {
        for (int j = 0; j < TAB_MAX_COL_NUM; j++) {
            bPlusTree[i][j] = nullptr;
        }
    }
}

bool IndexManager::isValid() {
    return valid;
}

IndexManager::~IndexManager() {
    for (int i = 0; i < DB_MAX_TABLE_NUM; i++) {
        for (int j = 0; j < TAB_MAX_COL_NUM; j++) {
            if (bPlusTree[i][j]) {
                bufPageManager->close();
                bufPageManager->fileManager->closeFile(fileIds[i][j]);
                delete bPlusTree[i][j];
                bPlusTree[i][j] = nullptr;
            }
        }
    }
}

int IndexManager::findEmptyIndex(int &emptyI, int &emptyJ) {
    for (int i = 0; i < DB_MAX_TABLE_NUM; i++) {
        for (int j = 0; j < TAB_MAX_COL_NUM; j++) {
            if (bPlusTree[i][j] == nullptr) {
                emptyI = i;
                emptyJ = j;
                return 0;
            }
        }
    }
    return -1;
}

BPlusTree* IndexManager::findIndex(const char* tableName, const char* indexName) {
    for (int i = 0; i < DB_MAX_TABLE_NUM; i++) {
        if (strcmp(tableNames[i], tableName) != 0) {
            continue;
        }
        for (int j = 0; j < TAB_MAX_COL_NUM; j++) {
            if (bPlusTree[i][j] && strcmp(indexNames[i][j], indexName) == 0) {
                return bPlusTree[i][j];
            }
        }
    }
    int emptyI, emptyJ;
    if (findEmptyIndex(emptyI, emptyJ) == -1) {
        printf("[ERROR] indexManager can accept no more index.\n");
        return nullptr;
    }
    strcpy(tableNames[emptyI], tableName);
    strcpy(indexNames[emptyI][emptyJ], indexName);
    char fileName[DB_MAX_NAME_LEN + TAB_MAX_NAME_LEN + TAB_MAX_NAME_LEN + 30];
    sprintf(fileName, "database/%s/%s_%s.index", databaseName, tableName, indexName);
    if (bufPageManager->fileManager->openFile(fileName, fileIds[emptyI][emptyJ])) {
        bPlusTree[emptyI][emptyJ] = new BPlusTree(fileIds[emptyI][emptyJ], bufPageManager, -1, -1);
        return bPlusTree[emptyI][emptyJ];
    }
    return nullptr;
}

int IndexManager::showIndex() {
    int cnt = 0;
    for (int i = 0; i < DB_MAX_TABLE_NUM; i++) {
        for (int j = 0; j < TAB_MAX_COL_NUM; j++) {
            if (bPlusTree[i][j]) {
                cnt++;
                printf("%s.%s\n", tableNames[i], indexNames[i][j]);
            }
        }
    }
    return cnt;
}

bool IndexManager::hasIndex(const char* tableName, const char* indexName) {
    char fileName[DB_MAX_NAME_LEN + TAB_MAX_NAME_LEN + TAB_MAX_NAME_LEN + 30];
    sprintf(fileName, "database/%s/%s_%s.index", databaseName, tableName, indexName);
    return (access(fileName, 0) != -1);
}

int IndexManager::initIndex(std::vector<std::string> indexTableNames, std::vector<std::vector<std::string>> indexColNames, std::vector<std::vector<uint16_t>> indexLens, std::vector<std::vector<uint8_t>> colTypes) {
    int tableSiz = indexTableNames.size();
    char fileName[DB_MAX_NAME_LEN + TAB_MAX_NAME_LEN + TAB_MAX_NAME_LEN + 30];
    for (int i = 0; i < tableSiz; i++) {
        for (int j = 0; j < indexColNames[i].size(); j++) {
            sprintf(fileName, "database/%s/%s_%s.index", databaseName, indexTableNames[i].c_str(), indexColNames[i][j].c_str());
            if (bufPageManager->fileManager->createFile(fileName)) {
                int fileId, emptyI, emptyJ;
                if (bufPageManager->fileManager->openFile(fileName, fileId)) {
                    if (findEmptyIndex(emptyI, emptyJ) == -1) {
                        printf("[ERROR] indexManager can accept no more index.\n");
                        return -1;
                    }
                    fileIds[emptyI][emptyJ] = fileId;
                    strcpy(tableNames[emptyI], indexTableNames[i].c_str());
                    strcpy(indexNames[emptyI][emptyJ], indexColNames[i][j].c_str());
                    bPlusTree[emptyI][emptyJ] = new BPlusTree(fileId, bufPageManager, indexLens[i][j], colTypes[i][j]);
                }
            }
        }
    }
    return 0;
}

int IndexManager::createIndex(const char* tableName, const char* indexName, uint16_t indexLen, uint8_t colType) {
    if (hasIndex(tableName, indexName)) {
        printf("[INFO] index %s already created.\n", indexName);
        return 0;
    }
    char fileName[DB_MAX_NAME_LEN + TAB_MAX_NAME_LEN + TAB_MAX_NAME_LEN + 30];
    sprintf(fileName, "database/%s/%s_%s.index", databaseName, tableName, indexName);
    if (bufPageManager->fileManager->createFile(fileName)) {
        int fileId, emptyI, emptyJ;
        if (bufPageManager->fileManager->openFile(fileName, fileId)) {
            if (findEmptyIndex(emptyI, emptyJ) == -1) {
                printf("[ERROR] indexManager can accept no more index.\n");
                return -1;
            }
            fileIds[emptyI][emptyJ] = fileId;
            strcpy(tableNames[emptyI], tableName);
            strcpy(indexNames[emptyI][emptyJ], indexName);
            bPlusTree[emptyI][emptyJ] = new BPlusTree(fileId, bufPageManager, indexLen, colType);
            return 0;
        }
    }
    return -1;
}

int IndexManager::removeIndex(const char* tableName, const char* indexName) {
    if (!hasIndex(tableName, indexName)) {
        printf("[ERROR] index %s has not been created.\n", indexName);
        return -1;
    }
    char fileName[DB_MAX_NAME_LEN + TAB_MAX_NAME_LEN + TAB_MAX_NAME_LEN + 30];
    sprintf(fileName, "database/%s/%s_%s.index", databaseName, tableName, indexName);
    if (bufPageManager->fileManager->removeFile(fileName)) {
        for (int i = 0; i < DB_MAX_TABLE_NUM; i++) {
            if (strcmp(tableNames[i], tableName) != 0) {
                continue;
            }
            for (int j = 0; j < TAB_MAX_COL_NUM; j++) {
                if (bPlusTree[i][j] && strcmp(indexNames[i][j], indexName) == 0) {
                    delete bPlusTree[i][j];
                    bPlusTree[i][j] = nullptr;
                    break;
                }
            }
        }
        return 0;
    }
    return -1;
}

int IndexManager::insert(const char* tableName, const char* indexName, void* data, const int val) {
    BPlusTree* cur = findIndex(tableName, indexName);
    if (cur == nullptr) {
        printf("[ERROR] index %s has not been created.\n", indexName);
        return -1;
    }
    cur->insert(data, val);
    return 0;
}

int IndexManager::insert(const char* tableName, const char* indexName, std::vector<void*> data, std::vector<int> val) {
    BPlusTree* cur = findIndex(tableName, indexName);
    if (cur == nullptr) {
        printf("[ERROR] index %s has not been created.\n", indexName);
        return -1;
    }
    if (data.size() != val.size()) {
        printf("[ERROR] size of data and val don't match.\n");
        return -1;
    }
    int siz = data.size();
    for (int i = 0; i < siz; i++) {
        cur->insert(data[i], val[i]);
    }
    return 0;
}

int IndexManager::search(const char* tableName, const char* indexName, void* data, std::vector<int> &res) {
    BPlusTree* cur = findIndex(tableName, indexName);
    if (cur == nullptr) {
        printf("[ERROR] index %s has not been created.\n", indexName);
        return -1;
    }
    cur->search(data, res);
    return 0;
}

int IndexManager::searchBetween(const char* tableName, const char* indexName, void* lData, void* rData, std::vector<int> &res, bool lIn, bool rIn) {
    BPlusTree* cur = findIndex(tableName, indexName);
    if (cur == nullptr) {
        printf("[ERROR] index %s has not been created.\n", indexName);
        return -1;
    }
    cur->searchBetween(lData, rData, res, lIn, rIn);
    return 0;
}

int IndexManager::update(const char* tableName, const char* indexName, void* data, int oldVal, int newVal) {
    BPlusTree* cur = findIndex(tableName, indexName);
    if (cur == nullptr) {
        printf("[ERROR] index %s has not been created.\n", indexName);
        return -1;
    }
    cur->update(data, oldVal, newVal);
    return 0;
}

int IndexManager::remove(const char* tableName, const char* indexName, void* data, int val) {
    BPlusTree* cur = findIndex(tableName, indexName);
    if (cur == nullptr) {
        printf("[ERROR] index %s has not been created.\n", indexName);
        return -1;
    }
    cur->remove(data, val);
    return 0;
}

void IndexManager::transform(const char* tableName, const char* indexName, int& val, int pageId, int slotId) {
    BPlusTree* cur = findIndex(tableName, indexName);
    cur->transform(val, pageId, slotId);
}

void IndexManager::transform(const char* tableName, const char* indexName, std::vector<int>& vals, std::vector<int> pageIds, std::vector<int> slotIds) {
    BPlusTree* cur = findIndex(tableName, indexName);
    for (int i = 0; i < vals.size(); i++) {
        cur->transform(vals[i], pageIds[i], slotIds[i]);
    }
}

void IndexManager::transformR(const char* tableName, const char* indexName, int val, int& pageId, int& slotId) {
    BPlusTree* cur = findIndex(tableName, indexName);
    cur->transformR(val, pageId, slotId);
}

void IndexManager::transformR(const char* tableName, const char* indexName, std::vector<int> vals, std::vector<int>& pageIds, std::vector<int>& slotIds) {
    BPlusTree* cur = findIndex(tableName, indexName);
    for (int i = 0; i < vals.size(); i++) {
        cur->transformR(vals[i], pageIds[i], slotIds[i]);
    }
}
