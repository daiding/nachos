#include "processmanager.h"

ProcessControlBlock::ProcessControlBlock(int processID, OpenFile* executableFile)
{
    processSpace = new AddrSpace(executableFile, processID);
    pid = processID;
    mainThread = NULL;
}
ProcessControlBlock::~ProcessControlBlock()
{
    DEBUG('a',"destroy the process %d", pid);
    delete processSpace;
}
ProcessManager::ProcessManager(int maxProcessNum)
{
    processTableMap = new BitMap(maxProcessNum);
    processTable = new ProcessControlBlock*[maxProcessNum];
}

ProcessManager::~ProcessManager()
{
    delete processTableMap;
    delete [] processTable;
}

ProcessControlBlock* ProcessManager::CreateProcess(OpenFile* executableFile)
{

    if (executableFile != NULL && processTableMap->NumClear() != 0)
    {
        int pid = processTableMap->Find();
        processTable[pid] = new ProcessControlBlock(pid,executableFile);
        return processTable[pid];
    }
    else
    {
        return NULL;
    }
    
}

void ProcessManager::ReleaseProcess(int pid)
{
    processTableMap->Clear(pid);
    delete processTable[pid];
    return;
}

TranslationEntry* ProcessManager::GetPageTable(int pid)
{
    if (processTableMap->Test(pid))
    {
        return processTable[pid]->GetProcessSpace()->GetPageTable();
    }
    else
    {
        return NULL;
    }
}

Thread* ProcessManager::GetMainThread(int pid)
{
    if (processTableMap->Test(pid))
    {
        return processTable[pid]->GetMainThread();
    }
    else
    {
        return NULL;
    }
}
