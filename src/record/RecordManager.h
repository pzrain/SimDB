#ifndef __RECORD_H_
#define __RECORD_H_
#include "../filesystem/bufmanager/BufPageManager.h"
#include "FileHandler.h"

class RecordManager{
private:
    BufPageManager* bufPageManager;
public:
    RecordManager(BufPageManager*);

    ~RecordManager();

    int createFile(const char* filename); 

    int removeFile(const char* filename);

    int openFile(const char* filename, FileHandler* fileHandler);// one file corresponds with one fileHandler

    int closeFile(FileHandler* fileHandler);
};

#endif