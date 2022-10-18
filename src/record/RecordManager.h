#ifndef __RECORD_H_
#define __RECORD_H_
#include "../filesystem/fileio/FileManager.h"
#include "FileHandler.h"

class RecordManager{
private:
    FileManager* fileManager;
public:
    RecordManager(FileManager*);

    ~RecordManager();

    void createFile(const char* filename); 

    void removeFile(const char* filename);

    void openFile(const char* filename, FileHandler* fileHandler);// one file corresponds with one fileHandler

    void closeFile(FileHandler* fileHandler);
};

#endif