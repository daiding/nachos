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
    int processNum;
    PhysicalMemoryManager* physicalMemoryManager;
    SwapDiskManager* swapDiskManager;
    int RequestPhysicalMemoryPage();//请求分配一个实页表
    public:
    MemoryManager();
    ~MemoryManager();
    int InitializeSwapPage(char *buffer, int size);
    void ProcessPageFault(int badVisualAddrNO);//处理缺页中断
};

#endif
