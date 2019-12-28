#include "memorymanager.h"
#include "system.h"
MemoryManager::MemoryManager()
{
    physicalMemoryManager = new PhysicalMemoryManager();
    swapDiskManager = new SwapDiskManager();
}

MemoryManager::~MemoryManager()
{
    delete physicalMemoryManager;
    delete [] swapDiskManager;
}



int MemoryManager::InitializeSwapPage(char* buffer, int size)
{
    return swapDiskManager->InitialVirtualMemoryPage(buffer,size);
}

void MemoryManager::ProcessPageFault(int badVirtualPageNO)
{
    stats->numPageFaults++;
    int processID = currentThread->space->GetProcessID();
    TranslationEntry* pageTable = processManager->GetPageTable(processID);
    DEBUG('a', "pid = %d, PAGE FAULT!\n", processID);
    int physicalPageNO = physicalMemoryManager->GetOneAvailablePage();
    while (physicalPageNO == -1)//内存空间已满，需要换出一个实页
    {
        int physicalPageToSwapOut = swapLRU->FindOnePageToSwap();
        int processIdOfSwapOutPage = physicalMemoryManager->GetProcessID(physicalPageToSwapOut);
        int virtualPageNoOfSwapOutPage = physicalMemoryManager->GetVirtualPageNO(physicalPageToSwapOut);
        TranslationEntry* thatProcessPageTable = processManager->GetPageTable(processIdOfSwapOutPage);
        thatProcessPageTable[virtualPageNoOfSwapOutPage].valid = false;
        thatProcessPageTable[virtualPageNoOfSwapOutPage].physicalPage = -1;
        if (thatProcessPageTable[virtualPageNoOfSwapOutPage].dirty)
        {
            swapDiskManager->SwapOut(physicalPageToSwapOut, thatProcessPageTable[virtualPageNoOfSwapOutPage].swapPage);
            thatProcessPageTable[virtualPageNoOfSwapOutPage].dirty = false;
        }
        physicalMemoryManager->ReleasePhysicalPage(physicalPageToSwapOut);
        physicalPageNO = physicalMemoryManager->GetOneAvailablePage();
        ASSERT(physicalPageToSwapOut == physicalPageNO);
        DEBUG('a', "Swap phsical page %d\n",physicalPageToSwapOut);
        printf("Swap page %d out\n",physicalPageNO);
    }
    swapDiskManager->SwapIn(pageTable[badVirtualPageNO].swapPage, physicalPageNO);
    printf("Swap page %d in\n",pageTable[badVirtualPageNO].swapPage);
    physicalMemoryManager->SetPageInformation(physicalPageNO, processID, badVirtualPageNO);
    pageTable[badVirtualPageNO].dirty = false;
    pageTable[badVirtualPageNO].valid = true;
    pageTable[badVirtualPageNO].physicalPage = physicalPageNO;
    DEBUG('a', "vitrul page NO: %d, physical page NO: %d\n", badVirtualPageNO,physicalPageNO);
    return;
}

void MemoryManager::ReleaseMemoryPages(TranslationEntry* pageTable, int numPage)
{
    for (int i = 0; i < numPage; i++)
    {
        swapDiskManager->ReleaseVirtualMemoryPage(pageTable[i].swapPage);
        physicalMemoryManager->ReleasePhysicalPage(pageTable[i].physicalPage);
    }
    return;
}



