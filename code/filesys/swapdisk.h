#ifndef SWAPDISK_H
#define SWAPDISK_H

#include "bitmap.h"
#include "synchdisk.h"

class SwapDisk
{
    public:
    SwapDisk(char* diskName);
    ~SwapDisk();
    int RequestVisualMemoryPage();//请求分配一个虚页
    void WriteAtVisualMemoryPage(int visualPageNumber, char *data, int dataSize);//往分配的虚拟存储控件写入数据
    void ReadAtVisualMemoryPage(int visualPageNumber, char *data, int dataSize);//
    bool VisualPageNumberIsValid(int visualPageNumber);//查询虚页号是否有效，有效返回true，否则返回false 
    void ReleaseVisualMemory();
    private:
    BitMap* swapDiskMemoryFrameMap;
    SynchDisk *synchDisk;
};
#endif
