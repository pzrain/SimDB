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

    int createFile(const char* filename); 

    int removeFile(const char* filename);

    int openFile(const char* filename, FileHandler* fileHandler);// one file corresponds with one fileHandler

    int closeFile(FileHandler* fileHandler);
};

#endif