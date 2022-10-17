#ifndef __FILEHANDLER_H__
#define __FILEHANDLER_H__

#include "RMComponent.h"
#include "FileManager.h"

class FileHandler{
private:
    FileManager* fileManager;
    int fileId;
public:
    FileHandler() {}

    FileHandler(FileManager* fileManager_, int fileId_);

    ~FileHandler();

    void init(FileManager* fileManager_, int fileId_);

    int getFileId();
    
    void getRecord(const int recordId, Record &record) const;

    void insertRecord(const char* recordData, int &recordId);

    void removeRecord(const int recordId);

    void updateRecord(const Record &record);

    void writeBack(const int pageNumber); // write page back to disk
};


#endif