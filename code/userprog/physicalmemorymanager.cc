#include "physicalmemorymanager.h"

PhysicalMemoryManager::PhysicalMemoryManager()
{
    physicalMemoryMap = new BitMap(PhysicalPageNum);
    physicalMemoryPageTable = new PhysicalPageEntity[PhysicalPageNum];
}

PhysicalMemoryManager::~PhysicalMemoryManager()
{
    delete physicalMemoryMap;
    delete physicalMemoryPageTable;
}

int PhysicalMemoryManager::GetProcessID(int physicalPageNO)
{
    return physicalMemoryPageTable[physicalPageNO].processID;
}

int PhysicalMemoryManager::GetVisualPageNO(int physicalPageNO)
{
    return physicalMemoryPageTable[physicalPageNO].visualPageNO;
}

int PhysicalMemoryManager::GetOneAvailablePage()
{
    if (physicalMemoryMap->NumClear() > 0)
    {
        int availablePage = physicalMemoryMap->Find();
        return availablePage;
    }
    else
    {
        return -1;
    }
}

void PhysicalMemoryManager::SetPageInformation(int physicalPageNO, int processID, int visualPageNO)
{
    physicalMemoryPageTable[physicalPageNO].processID = processID;
    physicalMemoryPageTable[physicalPageNO].visualPageNO = visualPageNO;
    return;
}

void PhysicalMemoryManager::ReleasePhysicalPage(int physicalPageNO)
{
    if (physicalPageNO > 0)
    {
        physicalMemoryMap->Clear(physicalPageNO);
    }
    return;
}