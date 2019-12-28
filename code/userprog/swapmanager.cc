#include "swapmanager.h"
#include "machine.h"
#include "system.h"

#define MAX_SWAP_PAGE 1024

SwapDiskManager::SwapDiskManager()
{
    swapDiskMap = new BitMap(MAX_SWAP_PAGE);
    fileSystem->Create("SWAPDISK",MAX_SWAP_PAGE * PageSize );
    swapDisk = fileSystem->Open("SWAPDISK");
}

SwapDiskManager::~SwapDiskManager()
{
    delete swapDiskMap;
    fileSystem->Remove("SWAPDISK");
}

int SwapDiskManager::InitialVirtualMemoryPage(char *data, int dataSize)
{
    if (swapDiskMap->NumClear() == 0)
    {
        DEBUG('a',"Swap Disk has no available page\n");
        return -1;
    }
    else
    {
        int swapPageNO = swapDiskMap->Find();
        if (PageSize > dataSize)
        {
            char* buffer;
            buffer = new char[PageSize];
            bzero(buffer,PageSize);
            bcopy(data, buffer, dataSize);
            swapDisk->WriteAt(data,dataSize,swapPageNO * PageSize);
            return swapPageNO;
        }
        else
        {
            swapDisk->WriteAt(data,dataSize,swapPageNO * PageSize);
            return swapPageNO;
        }
        stats->numDiskWrites++;
    }
}

void SwapDiskManager::SwapOut(int physicalPageNO,int swapPageNO)
{
    stats->numDiskWrites++;
    swapDisk->WriteAt(&(machine->mainMemory[physicalPageNO*PageSize]), PageSize,swapPageNO * PageSize);
    return;
}

void SwapDiskManager::SwapIn(int swapPageNo,int physicalPageNO)
{
    stats->numDiskReads++;
    swapDisk->ReadAt(&(machine->mainMemory[physicalPageNO*PageSize]), PageSize, swapPageNo * PageSize);
    return;
}

void SwapDiskManager::ReleaseVirtualMemoryPage(int pageNO)
{
    swapDiskMap->Clear(pageNO);
    return;
}



