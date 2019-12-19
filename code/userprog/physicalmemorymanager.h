#ifndef PHYSICALMEMORYMANAGER_H
#define PHYSICALMEMORYMANAGER_H

#include "bitmap.h"

#define PhysicalPageNum 32

class PhysicalPageEntity{
    public:
    int processID;
    int visualPageNO;
};

class PhysicalMemoryManager{
    private:
    BitMap* physicalMemoryMap;
    PhysicalPageEntity* physicalMemoryPageTable;
    public:
    PhysicalMemoryManager();
    ~PhysicalMemoryManager();
    int GetProcessID(int physicalPageNO);
    int GetVisualPageNO(int physicalPageNO);
    int GetOneAvailablePage();
    void SetPageInformation(int physicalPageNO, int processID, int visualPage);
    void ReleasePhysicalPage(int physicalPageNO);
};

#endif
