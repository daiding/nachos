#ifndef PHYSICALMEMORYMANAGER_H
#define PHYSICALMEMORYMANAGER_H

#include "bitmap.h"
#include "machine.h"

#define PhysicalPageNum NumPhysPages

class PhysicalPageEntity{
    public:
    int processID;
    int virtualPageNO;
};

class PhysicalMemoryManager{
    private:
    BitMap* physicalMemoryMap;
    PhysicalPageEntity* physicalMemoryPageTable;
    public:
    PhysicalMemoryManager();
    ~PhysicalMemoryManager();
    int GetProcessID(int physicalPageNO);
    int GetVirtualPageNO(int physicalPageNO);
    int GetOneAvailablePage();
    void SetPageInformation(int physicalPageNO, int processID, int virtualPage);
    void ReleasePhysicalPage(int physicalPageNO);
};

#endif
