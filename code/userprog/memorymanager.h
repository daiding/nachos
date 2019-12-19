#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H


#include "bitmap.h"
#include "translate.h"
#include "addrspace.h"
#include "swapmanager.h"
#include "machine.h"
#include "physicalmemorymanager.h"

#define MaxProcessNum 32

class MemoryManager
{
    private:
    int processNum;
    PhysicalMemoryManager* physicalMemoryManager;
    AddrSpace** processTable;
    SwapDiskManager* swapDiskManager;
    int RequestPhysicalMemoryPage();//请求分配一个实页表
    public:
    MemoryManager();
    ~MemoryManager();
    int InitializeSwapPage(char *buffer, int size);
    AddrSpace* CreateAddrSpace(OpenFile* executable);//为进程创建进程空间
    void ProcessPageFault(int badVisualAddrNO);//处理缺页中断
    AddrSpace* ShareAddrSpace();//创建新线程时调用，暂不实现
};

#endif
