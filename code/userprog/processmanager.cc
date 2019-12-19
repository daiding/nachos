#include "processmanager.h"


ProcessManager::ProcessManager(int maxProcessNum)
{
    processTableMap = new BitMap(maxProcessNum);
    processTable = new AddrSpace*[maxProcessNum];
}

ProcessManager::~ProcessManager()
{
    delete processTableMap;
    delete [] processTable;
}

AddrSpace* ProcessManager::CreateAddrSpace(OpenFile* executableFile)
{
    if (processTableMap->NumClear() != 0)
    {
        int pid = processTableMap->Find();
        processTable[pid] = new AddrSpace(executableFile,pid);
        return processTable[pid];
    }
    else
    {  
        return NULL;
    }
    
}

void ProcessManager::ReleaseAddrSpace(int pid)
{
    processTableMap->Clear(pid);
    delete processTable[pid];
    return;
}

TranslationEntry* ProcessManager::GetPageTable(int pid)
{
    if (processTableMap->Test(pid))
    {
        return processTable[pid]->GetPageTable();
    }
    else
    {
        return NULL;
    }
}