#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "bitmap.h"
#include "translate.h"
#include "addrspace.h"



class ProcessManager
{
    private:
    AddrSpace** processTable;
    BitMap* processTableMap;
    public:
    ProcessManager(int maxProcessNum);
    ~ProcessManager();
    AddrSpace* CreateAddrSpace(OpenFile* executableFile);//为进程创建进程空间
    void ReleaseAddrSpace(int pid);
    TranslationEntry* GetPageTable(int pid);
};

#endif