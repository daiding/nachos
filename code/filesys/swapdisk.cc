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

int SwapDisk::RequestVisualMemoryPage()
{
    int freeVisualPageNO;
    if (swapDiskMemoryFrameMap->NumClear() > 0)
    {
        freeVisualPageNO = swapDiskMemoryFrameMap->Find();
        if (!swapDiskMemoryFrameMap->Test(freeVisualPageNO))
        {
            swapDiskMemoryFrameMap->Mark(freeVisualPageNO);
        }
    }
    else 
    {
        return -1;
    }
    return freeVisualPageNO;
}

bool SwapDisk::VisualPageNumberIsValid(int visualPageNO)
{
    if (swapDiskMemoryFrameMap->Test(visualPageNO))
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

void SwapDisk::ReadAtVisualMemoryPage(int visualPageNumber, char *data, int dataSize)
{
    char *buffer = new char[BytesNumPerPage];
    bzero(buffer, BytesNumPerPage);
    synchDisk->ReadSector(visualPageNumber,buffer);
    bcopy(buffer, data, dataSize);
    return;
}

void SwapDisk::WriteAtVisualMemoryPage(int visualPageNumber, char *data, int dataSize)
{
    char *buffer = new char[BytesNumPerPage];
    bzero(buffer,BytesNumPerPage);
    bcopy(buffer, data, dataSize);
    return;
}
