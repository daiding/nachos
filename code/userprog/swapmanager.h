#ifndef SWAPMEMORYMANAGER_H
#define SWAPMEMORYMANAGER_H
#include "filesys.h"
#include "bitmap.h"


class SwapDiskManager
{
    public:
    SwapDiskManager();
    ~SwapDiskManager();
    void SwapIn(int swapPageNO, int physicalPageNO);//更新各处信息，将swap分区中的虚页移入内存中的实页
    void SwapOut(int physicalPageNO, int swapPageNO);//由MemoryManager类调用，该类应提供一个要换出的实页的页号，
    int InitialVirtualMemoryPage(char *processData, int dataSize);
    void ReleaseVirtualMemoryPage(int pageNO);
    private:
    OpenFile* swapDisk;
    BitMap* swapDiskMap;

};

#endif
