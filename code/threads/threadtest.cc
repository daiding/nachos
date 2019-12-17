// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "dllist-driver.h"
#include "table.h"
#include "BoundedBuffer.h"
#include "table-driver.h"
#include "BoundedBuffer-driver.h"

// set in main.cc
int testnum = 1;
int threadnum = 2;
int nodenum = 10;
int errornum = 0;
bool syncThreadMutexEnabled = false;
bool watcherThreadEnabled = false;

// Helper func
void ThreadSwitchErrnum(int expected)
{
    if (errornum == expected)
    {
        printf("[Yield] Thread switch as expected errornum = %d!\n", errornum);
        currentThread->Yield();
    }
}

namespace
{
DLList dlist;
SynchDLList sdlist;
Table table(10);
BoundedBuffer buffer(10);
}

void WatcherThread(int which)
{
    if(!watcherThreadEnabled)
    {
        return;
    }
    // 这个Watcher很丑，要等到调度器提供线程ID的时候才能改成正常的样子
    int watcher_counter = 0;
    int last_size = 0;
    while(1)
    {
        ++watcher_counter;
        if(dlist.Size() != last_size)
        {
            printf("[Watcher %d] dlist.Size() == %d\n", which, dlist.Size());
            last_size = dlist.Size();
            watcher_counter = 0;
        }
        else if(watcher_counter == 10000)
        {
            printf("[Watcher %d] Exit \n", which);
            break;
        }
        currentThread->Yield();
    }
}

void InsertAndRemoveThread(int which)
{
    printf("[INFO] Thread %d is inserting elements to dlist\n", which);
    if(errornum == 4|| errornum == 5)
    {
        DLListRandomInsertKeys(nodenum, &dlist, which);
    }
    else
    {
        DLListSequentialAppendKeys(nodenum, &dlist,which);
    }

    ThreadSwitchErrnum(1);
    currentThread->Yield();
    printf("[INFO] Thread %d is removing head elements of dlist\n", which);

    if(errornum == 5)
    {
        DLListRandomRemoveItems(nodenum, &dlist, which);
    }
    else
    {
        DLListRemoveHeadItems(nodenum, &dlist, which);
    }
}

//----------------------------------------------------------------------
// ThreadTestLab1
// Show a buggy concurrent action of Dllist class
//----------------------------------------------------------------------

void ThreadTestLab1()
{
    DEBUG('t', "Entering ThreadTestLab1 : Dllist concurrent problems");
    Thread* watcher = new Thread("watcher");
    watcher->Fork(WatcherThread, 0);
    for (int i = 0; i < threadnum; i++)
    {
        Thread *t = new Thread("forked thread");
        t->Fork(InsertAndRemoveThread, i);
    }
}

void SynchInsertAndRemoveThread(int which)
{
    static Lock mutex("SyncInsertRemoveMutex");
    if(syncThreadMutexEnabled)
    {
        printf("[Thread %d] Trying to acquire mutex\n", which);
        mutex.Acquire();
        printf("[Thread %d] Acquired mutex\n", which);
    }
    printf("[INFO] Thread %d is inserting elements to sdlist\n", which);
    if(errornum == 4|| errornum == 5)
    {
        SynchDLListRandomInsertKeys(nodenum, &sdlist, which);
    }
    else
    {
        SynchDLListSequentialAppendKeys(nodenum,&sdlist,which);
    }

    ThreadSwitchErrnum(1);
    currentThread->Yield();
    printf("[INFO] Thread %d is removing head elements of sdlist\n", which);
    if(errornum == 5)
    {
        SynchDLListRandomRemoveItems(nodenum, &sdlist, which);
    }
    else
    {
        SynchDLListRemoveHeadItems(nodenum, &sdlist, which);
    }
    if(syncThreadMutexEnabled)
    {
        printf("[Thread %d] Release mutex\n", which);
        mutex.Release();
    }
}

void SynchWatcherThread(int which)
{
    if(!watcherThreadEnabled)
    {
        return;
    }
    // 同理很丑，以后再改
    int watcher_counter = 0;
    int last_size = 0;
    while(1)
    {
        ++watcher_counter;
        if(sdlist.Size() != last_size)
        {
            printf("[Watcher %d] dlist.Size() == %d\n", which, sdlist.Size());
            last_size = sdlist.Size();
            watcher_counter = 0;
        }
        else if(watcher_counter == 10000)
        {
            printf("[Watcher %d] Exit \n", which);
            break;
        }
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTestLab2
// Fix the lab1 problems
//----------------------------------------------------------------------
void ThreadTestLab2()
{
    DEBUG('t', "Entering ThreadTest_Lab2 : SyncDllist");
    Thread* watcher = new Thread("watcher");
    watcher->Fork(SynchWatcherThread, 0);

    for (int i = 0; i < threadnum; i++)
    {
        // char * NAME = new char[12];
        // snprintf(NAME, 12, "Thread %d",i);
        Thread *t = new Thread("forked thread");
        t->Fork(SynchInsertAndRemoveThread, i);
    }
}

//----------------------------------------------------------------------
// ThreadTestLab3
// test the table
//----------------------------------------------------------------------
void TableInsertAndRemoveThread(int which)
{

	printf("[INFO] Thread %d is inserting  elements to table\n", which);
	tableInsert(table, 3, which);
	currentThread->Yield();
	printf("[INFO] Thread %d is removing elements of table\n", which);
	tableRemove(table,2,which);
}

void ThreadTestLab3()
{
	for (int i = 0; i < threadnum; i++)
	{
		Thread *t = new Thread("forked thread");
		t->Fork(TableInsertAndRemoveThread,i);
	}
}

//----------------------------------------------------------------------
// ThreadTestLab4
// test the boundedBuffer
//----------------------------------------------------------------------
void BufferWriteAndReadThread(int which)
{
    printf("[INFO] Thread %d is writing\n", which);
	BufferWrite(buffer,13,which);     //buffer的大小是10
	currentThread->Yield();
	printf("[INFO] Thread %d is reading\n", which);
	BufferRead(buffer,7,which);
}
void ThreadTestLab4()
{
    for (int i = 0; i < threadnum; i++)
	{
		Thread *t = new Thread("forked thread");
		t->Fork(BufferWriteAndReadThread,i);
	}
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void ThreadTest()
{
    printf("[ThreadTest Start] Given testnum = %d, threadnum = %d, nodenum = %d, errornum = %d\n",testnum, threadnum, nodenum, errornum);
    switch (testnum)
    {
    case 1:
        ThreadTestLab1();
        break;
    case 2:
        ThreadTestLab2();
        break;
	case 3:
		ThreadTestLab3();
		break;
	case 4:
		ThreadTestLab4();
    default:
        printf("No test specified.\n");
        break;
    }
}
