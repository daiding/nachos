#include "memorymanager.h"
#include "system.h"
MemoryManager::MemoryManager()
{
    processNum = 0;
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
    return swapDiskManager->InitialVisualMemoryPage(buffer,size);
}

void MemoryManager::ProcessPageFault(int badVisualPageNO)
{
    int processID = currentThread->space->GetProcessID();
    TranslationEntry* pageTable = processManager->GetPageTable(processID);
    int physicalPageNO = physicalMemoryManager->GetOneAvailablePage();
    while (physicalPageNO == -1)//内存空间已满，需要换出一个实页
    {
        int physicalPageToSwapOut = swapLRU->FindOnePageToSwap();
        int processIdOfSwapOutPage = physicalMemoryManager->GetProcessID(physicalPageToSwapOut);
        int visualPageNoOfSwapOutPage = physicalMemoryManager->GetVisualPageNO(physicalPageToSwapOut);
        TranslationEntry* thatProcessPageTable = processManager->GetPageTable(processIdOfSwapOutPage);
        thatProcessPageTable[visualPageNoOfSwapOutPage].valid = false;
        thatProcessPageTable[visualPageNoOfSwapOutPage].physicalPage = -1;
        if (thatProcessPageTable[visualPageNoOfSwapOutPage].dirty)
        {
            swapDiskManager->SwapOut(physicalPageToSwapOut, thatProcessPageTable[visualPageNoOfSwapOutPage].swapPage);
            thatProcessPageTable[visualPageNoOfSwapOutPage].dirty = false;
        }
        physicalMemoryManager->ReleasePhysicalPage(physicalPageToSwapOut);
        physicalPageNO = physicalMemoryManager->GetOneAvailablePage();
    }
    swapDiskManager->SwapIn(pageTable[badVisualPageNO].swapPage, physicalPageNO);
    physicalMemoryManager->SetPageInformation(physicalPageNO, processID, badVisualPageNO);
    pageTable[badVisualPageNO].dirty = false;
    pageTable[badVisualPageNO].valid = true;
    pageTable[badVisualPageNO].physicalPage = physicalPageNO;
    DEBUG('a', "physical page NO: %d, vitrul page NO: %d\n", badVisualPageNO,physicalPageNO);
    return;
}



