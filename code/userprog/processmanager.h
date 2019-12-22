#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "list.h"
#include "thread.h"
#include "bitmap.h"
#include "translate.h"
#include "addrspace.h"

class ProcessControlBlock
{
    private:
    AddrSpace* processSpace;
    int pid;
    //ProcessControlBlock* parentProcess;
    Thread* mainThread;
    //List* activeChileProcess;
    //List* exitedChildProcess;
    //int exitStatus;
    public:
    ProcessControlBlock(int processID, OpenFile* executableFile);
    ~ProcessControlBlock();
    int GetProcessID(){return pid;}
    //void SetParentProcess(int parentProcessID);
    //ProcessControlBlock* GetParentProcessID(){return parentProcess;}
    Thread* GetMainThread(){return mainThread;}
    void SetMainThread(Thread* thread){
        mainThread = thread;
        thread->space = processSpace;
        return;
    }
    //int GetExitStatus(){return exitStatus;}
    //void SetExitStatus(int status)
    //{
        //exitStatus = status;
        //return;
    //}
    AddrSpace* GetProcessSpace(){return processSpace;}
    //void ExecChildProcess(ProcessControlBlock* childPCB);
    //void ExitChildProcess(ProcessControlBlock* childPCB);
};


class ProcessManager
{
    private:
    ProcessControlBlock** processTable;
    BitMap* processTableMap;
    public:
    ProcessManager(int maxProcessNum);
    ~ProcessManager();
    ProcessControlBlock* CreateProcess(OpenFile* executableFile);//为进程创建进程空间
    void ReleaseProcess(int pid);
    TranslationEntry* GetPageTable(int pid);
    Thread* GetMainThread(int pid);
};

#endif