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
#include "Elevator.h"
#include <cstdarg>
#include <ctime>
// 获得标识符的字符串（方便重构工具重命名，类似于C#的nameof运算符）
#define NAMEOF(Identifier) #Identifier

// 一些全局变量，在main.cc中设定
int elevatorNum = 5;
int riderNum = 100;
int floorsNum = 5;
int testnum = 1;
int threadnum = 2;
int nodenum = 10;
int errornum = 0;
bool syncThreadMutexEnabled = false;
bool watcherThreadEnabled = false;

// 无名命名空间（内部链接）
namespace
{
Lock aliveTestThreadCountLock(NAMEOF(aliveTestThreadCountLock));
int aliveThreadCount = 0; // 为什么不用Semaphore？因为我想读它的具体值……
Lock startupTestThreadCountLock(NAMEOF(startupTestThreadCountLock));
int startupThreadCount = 0;

DLList dlist;
SynchDLList sdlist;
Table *table = NULL;
BoundedBuffer *boundedBuffer = NULL;

EventBarrier eventBarrier;

Building *building = NULL;
Elevator *elevators = NULL; //array
} // namespace

/* Helper functions START */

// Yield currentThread accroding to expected errornum
void ThreadSwitchErrnum(int expected)
{
    if (errornum == expected)
    {
        printf("[Yield] Thread switch as expected errornum = %d!\n", errornum);
        currentThread->Yield();
    }
}

// Execute threadFunc and print debug messages
void ThreadTestHelper(VoidFunctionPtr threadFunc, char flag, char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    DEBUG(flag, format, ap);
    va_end(ap);
    for (int i = 0; i < threadnum; i++)
    {
        Thread *t = new Thread("forked thread");
        t->Fork(threadFunc, i);
    }
}

// aliveThreadCount = threadnum atomly

void InitAliveTestThreadCount(int num = threadnum)
{
    aliveTestThreadCountLock.Acquire();
    aliveThreadCount = num;
    aliveTestThreadCountLock.Release();
}

// aliveThreadCount-- atomly
void DecreaseAliveTestThreadCount()
{
    aliveTestThreadCountLock.Acquire();
    aliveThreadCount--;
    aliveTestThreadCountLock.Release();
}

// 顾名思义 暂时替代没有Thread::Join做清理很麻烦的问题（避免内存泄漏）
void WaitTestThreadsComplete()
{
    for (;;)
    {
        aliveTestThreadCountLock.Acquire();
        if (aliveThreadCount != 0)
        {
            aliveTestThreadCountLock.Release();
            currentThread->Yield();
        }
        else
        {
            aliveTestThreadCountLock.Release();
            break;
        }
    }
}

// 观察者辅助函数 用来观察数据结构的大小变化 方便调试和验证
// 假定int T::Size()存在, C++98 写SFINAE太恶心了，就这样凑合吧，编译错误的话检查一下T::Size是否存在
// 观察者通过 -W 选项开启
template <typename T>
void WatcherHelper(int which, T *target, const char *targetIdentifier)
{
    // 这个Watcher很丑，要等到调度器提供线程标识或者更多信息，以及Fork允许更多参数的函数的时候才能改成正常的样子
    if (!watcherThreadEnabled)
    {
        return;
    }
    int lastSize = -1;
    bool first = true;
    for (;;)
    {
        if (target->Size() != lastSize)
        {
            printf("[Watcher %d] %s.Size() == %d\n", which, targetIdentifier, target->Size());
            lastSize = target->Size();
            currentThread->Yield();
        }
        else
        {
            if (first)
            {
                first = false;
                currentThread->Yield();
            }
            else
            {
                aliveTestThreadCountLock.Acquire();
                if (aliveThreadCount != 0)
                {
                    aliveTestThreadCountLock.Release();
                    currentThread->Yield();
                }
                else
                {
                    aliveTestThreadCountLock.Release();
                    printf("[Watcher %d] Exit \n", which);
                    break;
                }
            }
        }
    }
}

/* Helper functions END */

// 辅助类，用于debugName的动态字符串的垃圾回收（没办法，nachos 所有的类 的debugName竟然都直接浅复制）
class TempCStringCollector
{
public:
    TempCStringCollector()
    {
        pool = new List;
    }
    ~TempCStringCollector()
    {
        char *temp;
        while ((temp = (char *)pool->Remove()))
        {
            delete temp;
        }
        delete pool;
    }
    // WARNING: DON'T DEALLOC sharedpointer MANUALLY
    void Add(char *sharedpointer)
    {
        pool->Append(sharedpointer);
    }

private:
    List *pool;
};

//----------------------------------------------------------------------
// ThreadTestLab1
// Show a buggy concurrent action of Dllist class
//----------------------------------------------------------------------
void DlistWatcherThread(int which)
{
    WatcherHelper(which, &dlist, NAMEOF(dlist));
}

void InsertAndRemoveThread(int which)
{
    printf("[INFO] Thread %d is inserting elements to dlist\n", which);
    if (errornum == 4 || errornum == 5)
    {
        DLListRandomInsertKeys(nodenum, &dlist, which);
    }
    else
    {
        DLListSequentialPrependKeys(nodenum, &dlist, which);
    }

    ThreadSwitchErrnum(1);
    // currentThread->Yield();
    printf("[INFO] Thread %d is removing head elements of dlist\n", which);

    if (errornum == 5)
    {
        DLListRandomRemoveItems(nodenum, &dlist, which);
    }
    else
    {
        DLListRemoveHeadItems(nodenum, &dlist, which);
    }
    DecreaseAliveTestThreadCount();
}

void ThreadTest1Dllist()
{
    InitAliveTestThreadCount();
    Thread *watcher = new Thread(NAMEOF(watcher));
    watcher->Fork(DlistWatcherThread, 0);
    ThreadTestHelper(InsertAndRemoveThread, 't', "Entering ThreadTestLab1 : Dllist concurrent problems");
}

//----------------------------------------------------------------------
// ThreadTestLab2
// Fix the problems of lab1
// Pass -M option to fix errornum = 1~3
//----------------------------------------------------------------------

void SdlistWatcherThread(int which)
{
    WatcherHelper(which, &sdlist, NAMEOF(sdlist));
}

void SynchInsertAndRemoveThread(int which)
{
    static Lock mutex("SyncInsertRemoveMutex");
    if (syncThreadMutexEnabled)
    {
        printf("[Thread %d] Trying to acquire mutex\n", which);
        mutex.Acquire();
        printf("[Thread %d] Acquired mutex\n", which);
    }
    printf("[INFO] Thread %d is inserting elements to sdlist\n", which);
    if (errornum == 4 || errornum == 5)
    {
        SynchDLListRandomInsertKeys(nodenum, &sdlist, which);
    }
    else
    {
        SynchDLListSequentialPrependKeys(nodenum, &sdlist, which);
    }

    ThreadSwitchErrnum(1);
    // currentThread->Yield();
    printf("[INFO] Thread %d is removing head elements of sdlist\n", which);

    if (errornum == 5)
    {
        SynchDLListRandomRemoveItems(nodenum, &sdlist, which);
    }
    else
    {
        SynchDLListRemoveHeadItems(nodenum, &sdlist, which);
    }
    if (syncThreadMutexEnabled)
    {
        printf("[Thread %d] Release mutex\n", which);
        mutex.Release();
    }
    DecreaseAliveTestThreadCount();
}

void ThreadTest2SyncDllist()
{
    InitAliveTestThreadCount();
    Thread *watcher = new Thread("watcher");
    watcher->Fork(SdlistWatcherThread, 0);
    ThreadTestHelper(SynchInsertAndRemoveThread, 't', "Entering ThreadTest_Lab2 : SyncDllist");
}

//----------------------------------------------------------------------
// ThreadTestLab3
// Test the table
//----------------------------------------------------------------------
void TableInsertAndRemoveThread(int which)
{
    printf("[INFO] Thread %d is inserting elements to table\n", which);
    TableInsert(table, nodenum, which);
    currentThread->Yield();
    printf("[INFO] Thread %d is removing elements of table\n", which);
    TableRemove(table, nodenum, which);
    DecreaseAliveTestThreadCount();
}

void ThreadTest3Table()
{
    InitAliveTestThreadCount();
    table = new Table(nodenum);
    ThreadTestHelper(TableInsertAndRemoveThread, 't', "Entering ThreadTestLab3 : Table");
    WaitTestThreadsComplete();
    delete table;
}

//----------------------------------------------------------------------
// ThreadTestLab4
// Test BoundedBuffer
//----------------------------------------------------------------------
void BufferWriteThread(int which)
{
    printf("[INFO] Thread %d is writing bytes to buffer\n", which);
    BufferWrite(boundedBuffer, boundedBuffer->Size(), which);
    DecreaseAliveTestThreadCount();
}

void BufferReadThread(int which)
{
    printf("[INFO] Thread %d is reading bytes to buffer\n", which);
    BufferRead(boundedBuffer, boundedBuffer->Size(), which);
    DecreaseAliveTestThreadCount();
}

void ThreadTest4BoundedBuffer()
{
    DEBUG('t', "Entering ThreadTestLab4 : BoundBuffer");
    printf("Notice : In this test, threadnum will be multiplied by 2 (as %d)\n", threadnum * 2);
    RandomInit(time(NULL));
    InitAliveTestThreadCount(threadnum * 2);
    boundedBuffer = new BoundedBuffer(nodenum);
    for (int i = 0; i < threadnum; i++)
    {
        Thread *t = new Thread("consumer");
        t->Fork(BufferReadThread, i);
    }
    for (int i = threadnum; i < 2 * threadnum; i++)
    {
        Thread *t = new Thread("producer");
        t->Fork(BufferWriteThread, i);
    }
    WaitTestThreadsComplete();
    delete boundedBuffer;
}
//----------------------------------------------------------------------
// ThreadTest5
// 	EventBarrier
//----------------------------------------------------------------------

void EventWaiterThread(int which)
{
    startupTestThreadCountLock.Acquire();
    startupThreadCount++;
    startupTestThreadCountLock.Release();
    printf("[EventWaiterThread %d] Waiting for an event(1)\n", which);
    eventBarrier.Wait();
    printf("[EventWaiterThread %d] Received signal\n", which);
    eventBarrier.Complete();
    printf("[EventWaiterThread %d] Passed barrier (1)\n", which);
    // threadtest.cc的EventWaiterThread的被注释代码（问题详见EventBarrier.cc）
    // currentThread->Yield();
    // currentThread->Yield();
    // printf("[EventWaiterThread %d] Waiting for an already happened event(2), should pass right now\n", which);
    // eventBarrier.Wait();
    // eventBarrier.Complete();
    // printf("[EventWaiterThread %d] Pass barrier (2)\n", which);
    DecreaseAliveTestThreadCount();
}

void EventSignallerThread(int which)
{
    printf("[EventSignallerThread %d] Singalling (1)\n", which);
    eventBarrier.Signal();
    printf("[EventSignallerThread %d] Signal received by all waiters, pass barrier.\n", which);
    printf("[EventSignallerThread %d] Singal again\n", which);
    eventBarrier.Signal();
    printf("[EventSignallerThread %d] Passed\n", which);
    // threadtest.cc的EventWaiterThread的被注释代码（问题详见EventBarrier.cc）
    // printf("[EventSignallerThread %d] Let's signal again(2). \n", which);
    // eventBarrier.Signal();
    // printf("[EventSignallerThread %d] Pass barrier(2)!\n", which);
    DecreaseAliveTestThreadCount();
}

void ThreadTest5EventBarrier()
{
    DEBUG('t', "Entering ThreadTest5 : EventBarrier");
    printf("Notice : In this test, threadnum will be add 1 (as %d)\n", threadnum + 1);
    RandomInit(time(NULL));

    puts("[INFO] Ensure Signaller is execuated after Waiters");
    InitAliveTestThreadCount(threadnum + 1);
    for (int i = 0; i < threadnum; i++)
    {
        Thread *waiter = new Thread(NAMEOF(waiter));
        waiter->Fork(EventWaiterThread, i);
    }
    currentThread->Yield();
    for (;;)
    {
        startupTestThreadCountLock.Acquire();
        if (startupThreadCount == threadnum)
        {
            startupTestThreadCountLock.Release();
            break;
        }
        else
        {
            startupTestThreadCountLock.Release();
            currentThread->Yield();
        }
    }
    Thread *signaller = new Thread(NAMEOF(signaller));
    signaller->Fork(EventSignallerThread, threadnum);
    WaitTestThreadsComplete();
}

void AlarmTestThread(int which)
{
    int howLong = Random() % 100;
    int sleepTime = stats->totalTicks;
    printf("[AlarmTestThread %d] Pause for %d TimerTicks, sleep at %d, awoken at approx. %d\n", which, howLong, sleepTime, sleepTime + howLong * TimerTicks);
    alarmer->Pause(howLong);
    int awokenTime = stats->totalTicks;
    printf("[AlarmTestThread %d] Awoken now at %d! Late for %d ticks\n", which, awokenTime, awokenTime - (sleepTime + howLong * TimerTicks));
}

//----------------------------------------------------------------------
// ThreadTest6
// 	AlarmClock
//----------------------------------------------------------------------
void ThreadTest6AlarmClock()
{
    if (alarmer == NULL)
    {
        fprintf(stderr, "Error : Enable Global Timer AND Alarm first (Are you missing -rs option?)\n");
        currentThread->Finish();
        Exit(1);
    }
    ThreadTestHelper(AlarmTestThread, 't', "Entering ThreadTest6 : AlarmClock");
}
//----------------------------------------------------------------------
// ThreadTest7
// Elevator
// ---------------------------------------------------------------------
void ElevatorThread(int id)
{
    printf("[Elevator %d] Start\n", id);
    DEBUG('t', "Elevator %d start running\n", id);
    elevators[id].Run();
    DecreaseAliveTestThreadCount();
}

void rider(int id, int srcFloor, int dstFloor)
{
    Elevator *e;

    if (srcFloor == dstFloor)
    {
        printf("[Rider %d] Already here\n", id);
        return;
    }
    DEBUG('t', "Rider %d travelling from %d to %d\n", id, srcFloor, dstFloor);
    int tryTime = 0;
    do
    {
        printf("[Rider %d][Floor %d] To get an elevator to go, I have failed %d time(s)\n", id, srcFloor, tryTime);
        tryTime++;
        if (srcFloor < dstFloor)
        {
            DEBUG('t', "Rider %d CallUp(%d)\n", id, srcFloor);
            building->CallUp(srcFloor);
            DEBUG('t', "Rider %d AwaitUp(%d)\n", id, srcFloor);
            e = building->AwaitUp(srcFloor);
        }
        else
        {
            DEBUG('t', "Rider %d CallDown(%d)\n", id, srcFloor);
            building->CallDown(srcFloor);
            DEBUG('t', "Rider %d AwaitDown(%d)\n", id, srcFloor);
            e = building->AwaitDown(srcFloor);
        }
        DEBUG('t', "Rider %d Enter()\n", id);
    } while (!e->Enter()); // elevator might be full!
    printf("[Rider %d][Floor %d] Entered elevator %d . Requesting floor %d\n", id, srcFloor, e->GetID(), dstFloor);
    DEBUG('t', "Rider %d RequestFloor(%d)\n", id, dstFloor);
    e->RequestFloor(dstFloor); // doesn't return until arrival
    ASSERT(e->GetCurrentFloor() == dstFloor);
    printf("[Rider %d][Floor %d] Arrived. Exiting...\n", id, e->GetCurrentFloor());
    DEBUG('t', "Rider %d Exit()\n", id);
    e->Exit();
    printf("[Rider %d] Exited\n", id);
    DEBUG('t', "Rider %d finished\n", id);
}

void RiderThread(int id)
{
    while (true)
    {
        int srcFloor = Random() % floorsNum;
        int dstFloor = Random() % floorsNum;
        printf("[Rider %d] Going %d -> %d\n", id, srcFloor, dstFloor);
        rider(id, srcFloor, dstFloor);
        printf("[Rider %d] Finished %d -> %d\n", id, srcFloor, dstFloor);
        // currentThread->Yield();
        // alarmer->Pause(100);
    }
    DecreaseAliveTestThreadCount();
}

void ThreadTest7Elevator()
{
//    TempCStringCollector collector; // 结束时释放所有堆分配的debugName
    building = new Building("building", floorsNum, elevatorNum);
    elevators = building->GetElevator();
    InitAliveTestThreadCount(elevatorNum + riderNum);
    for (int i = 0; i < elevatorNum; i++)
    {
        char *c = new char[10];
        sprintf(c, "elevator%d", i);
//        collector.Add(c);
        Thread *t = new Thread(c);
        t->Fork(ElevatorThread, i);
    }
    for (int i = 0; i < riderNum; i++)
    {
        char *c = new char[10];
        sprintf(c, "rider%d", i);
//        collector.Add(c);
        Thread *t = new Thread(c);
        t->Fork(RiderThread, i);
    }
    // 注释行是保证没有守护进程的测试时使用的，如果全部睡死，就会停机，虽然这会导致char*的有意泄漏……
//    WaitTestThreadsComplete(); // 这里必须等待结束，否则collector会被提前释放掉
}
//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void ThreadTest()
{
    printf("[ThreadTest Start] Given testnum = %d, threadnum = %d, nodenum = %d, errornum = %d\n"
           "[ThreadTest Start] (if needed) elevatorNum = %d, riderNum = %d, floorsNum = %d\n",
           testnum, threadnum, nodenum, errornum, elevatorNum, riderNum, floorsNum);
    switch (testnum)
    {
    case 1:
        ThreadTest1Dllist(); // Dllist
        break;
    case 2:
        ThreadTest2SyncDllist(); // SyncDllist
        break;
    case 3:
        ThreadTest3Table(); // Table
        break;
    case 4:
        ThreadTest4BoundedBuffer(); // BoundedBuffer
        break;
    case 5:
        ThreadTest5EventBarrier();
        break;
    case 6:
        ThreadTest6AlarmClock();
        break;
    case 7:
        ThreadTest7Elevator();
        break;
    default:
        printf("No test specified.\n");
        break;
    }
}
