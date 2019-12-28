#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H


#include "bitmap.h"
#include "translate.h"
#include "addrspace.h"
#include "swapmanager.h"
#include "machine.h"
#include "physicalmemorymanager.h"



class MemoryManager
{
    private:
    PhysicalMemoryManager* physicalMemoryManager;
    SwapDiskManager* swapDiskManager;
    public:
    MemoryManager();
    ~MemoryManager();
    int InitializeSwapPage(char *buffer, int size);
    void ReleaseMemoryPages(TranslationEntry* pageTable, int pageNO);
    void ProcessPageFault(int badVirtualAddrNO);//处理缺页中断
};

#endif
