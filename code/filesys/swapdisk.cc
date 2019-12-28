#include "swapdisk.h"
#include "strings.h"
#define PageNumber NumSectors
#define BytesNumPerPage 32

SwapDisk::SwapDisk(char *diskName)
{
    swapDiskMemoryFrameMap = new BitMap(PageNumber);
    synchDisk = new SynchDisk(diskName);
}

SwapDisk::~SwapDisk()
{
    delete swapDiskMemoryFrameMap;
    delete synchDisk;
}

int SwapDisk::RequestVirtualMemoryPage()
{
    int freeVirtualPageNO;
    if (swapDiskMemoryFrameMap->NumClear() > 0)
    {
        freeVirtualPageNO = swapDiskMemoryFrameMap->Find();
        if (!swapDiskMemoryFrameMap->Test(freeVirtualPageNO))
        {
            swapDiskMemoryFrameMap->Mark(freeVirtualPageNO);
        }
    }
    else 
    {
        return -1;
    }
    return freeVirtualPageNO;
}

bool SwapDisk::VirtualPageNumberIsValid(int virtualPageNO)
{
    if (swapDiskMemoryFrameMap->Test(virtualPageNO))
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

void SwapDisk::ReadAtVirtualMemoryPage(int virtualPageNumber, char *data, int dataSize)
{
    char *buffer = new char[BytesNumPerPage];
    bzero(buffer, BytesNumPerPage);
    synchDisk->ReadSector(virtualPageNumber,buffer);
    bcopy(buffer, data, dataSize);
    return;
}

void SwapDisk::WriteAtVirtualMemoryPage(int virtualPageNumber, char *data, int dataSize)
{
    char *buffer = new char[BytesNumPerPage];
    bzero(buffer,BytesNumPerPage);
    bcopy(buffer, data, dataSize);
    return;
}
