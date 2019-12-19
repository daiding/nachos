#include "Alarm.h"
#include "system.h"
#include <limits.h>

__attribute__((unused)) static void DummyThread(int dummy)
{
    // Ensure nachos will not exit when there is no thread is ready and there are threads waiting for Alarm.
    // Update: Test whether it's right to add `&& alarmer->GetWaiterCount() == 0` in line 328, interrrupt.cc
    // if it's right, there is no need for this dummy thread
    while (alarmer->GetWaiterCount() != 0)
    {
        currentThread->Yield();
    }
}

Alarm::Alarm()
{
    waiterCount = 0;
    queue = new List;
}

Alarm::~Alarm()
{
    delete queue;
}

void Alarm::Pause(int howLong)
{
    ASSERT(howLong >= 0);
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    int endTime = stats->totalTicks + howLong * TimerTicks;
    waiterCount++;
    // if (waiterCount == 1)
    // {
    //     currentThread->Fork(DummyThread, 0);
    // }
    // Ensure nachos will not exit when there are no thread is ready.
    queue->SortedInsert((void *)currentThread, endTime);
    currentThread->Sleep();
    (void)interrupt->SetLevel(oldLevel); // re-enable interrupts
}

void Alarm::AwakeTimeoutThreads()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    int oldCount = waiterCount;
    for (int i = 0; i < oldCount; i++)
    {
        int endTime;
        Thread *waiter = (Thread *)queue->SortedRemove(&endTime);
        if (endTime <= stats->totalTicks)
        {
            scheduler->ReadyToRun(waiter);
            --waiterCount;
        }
        else
        {
            // queue在这里的操作只涉及SortedRemove和SortedInsert
            // 也就是假定是一个SortedList
            // 发现最小的还要等，马上退出循环
            queue->SortedInsert(waiter, endTime);
            break;
        }
    }
    (void)interrupt->SetLevel(oldLevel);
}
