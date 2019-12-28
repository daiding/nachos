#ifndef SWAPDISK_H
#define SWAPDISK_H

#include "bitmap.h"
#include "synchdisk.h"

class SwapDisk
{
    public:
    SwapDisk(char* diskName);
    ~SwapDisk();
    int RequestVirtualMemoryPage();//请求分配一个虚页
    void WriteAtVirtualMemoryPage(int virtualPageNumber, char *data, int dataSize);//往分配的虚拟存储控件写入数据
    void ReadAtVirtualMemoryPage(int virtualPageNumber, char *data, int dataSize);//
    bool VirtualPageNumberIsValid(int virtualPageNumber);//查询虚页号是否有效，有效返回true，否则返回false 
    void ReleaseVirtualMemory();
    private:
    BitMap* swapDiskMemoryFrameMap;
    SynchDisk *synchDisk;
};
#endif
