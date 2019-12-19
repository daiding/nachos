// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch-sem.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    while (value == 0) { 			// semaphore not available
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    }
    value--; 					// semaphore available,
    // consume its value

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
Lock::Lock(char* debugName)
{
    name = debugName;
    holderThread = NULL;
    lockSelf = new Semaphore(debugName, 1);
}

Lock::~Lock()
{
    delete lockSelf;
}

void Lock::Acquire()
{
    ASSERT(!isHeldByCurrentThread());
    // printf("[Lock %s] Waiting for lock...\n", name);
    lockSelf->P();
    // printf("[Lock %s] Locked\n", name);
    holderThread = currentThread;
}

void Lock::Release()
{
    ASSERT(isHeldByCurrentThread());
    holderThread = NULL;
    // printf("[Lock %s] Released\n", name);
    lockSelf->V();
}

bool Lock::isHeldByCurrentThread()
{
    return holderThread == currentThread;
}

bool Lock::isAvailable()
{
    return holderThread == NULL;
}

Condition::Condition(char* debugName)
{
    name = debugName;
    semSignallerBlock = new Semaphore(debugName, 0);
    semWaiters = new Semaphore(debugName, 0);
    semWaitersCountMutex = new Semaphore(debugName, 1);
    waitersCount = 0;
}
Condition::~Condition()
{
    delete semWaiters;
    delete semSignallerBlock;
    delete semWaitersCountMutex;
}
void Condition::Wait(Lock* conditionLock)
{
    ASSERT(conditionLock->isHeldByCurrentThread());

    // 如果只使用信号量实现，那么还要维护等待计数，而等待计数还需要额外的保护，这会造成额外的上下文切换
    // 而且为了保证广播逻辑的正确性，实现相当麻烦
    // 这里的实现参考了https://www.microsoft.com/en-us/research/wp-content/uploads/2004/12/ImplementingCVs.pdf 的最终实现版本
    // 但是，根据论文的说法，
    // 只用信号量实现条件变量可以得到正确语义的结果，但是性能不行（两次切换开销对于核心态够大了）

    // 网络上广为流传的版本存在着许多漏洞，例如
    // 1. 等待计数出现竞争条件
    // 包括助教提供的版本也存在这个问题，其信号量只保护了Wait部分的等待计数互斥
    // 如果在锁被释放后出现Signal和Broadcast的调用，将出现语义错误
    //
    // 2. 下方标注 论文中提到的1处的区域有很多问题
    // 论文中的举例：
    // 假设7个线程都调用了Condition::Wait()在1处挂起（1处已经释放锁，所以这是可能的）
    // 那么假定此时有一个线程调用了Condition::Broadcast()，那么将调用semWaiters->V()7次，使得semWaiters.Count == 7
    // 如果此时7个线程继续，一切都好. 但是，如果此时又有一个线程调用了Condition::Wait()
    // 那么那个线程会导致semWaiters.Count -= 1，并跳过Wait
    // 剩余7个线程中会有一个被加入semWaiters的等待队列
    // 这违背了条件变量的语义
    // 注意Condition::Signal存在相同的问题
    // 再加一个锁不是不行，但是实现进一步复杂化，影响性能，还有潜在死锁的风险

    // 等待计数：
    // 用来保证语义的正确，防止Condition::Signal()在无线程阻塞在Condition::Wait()的时候调用，
    // 出现 semWaiters.Count == 1，导致下一个Condition::Wait()的调用者直接跳过的问题

    // 引入等待计数后，它需要保护，故引入semCount
    semWaitersCountMutex->P();
    waitersCount++;
    semWaitersCountMutex->V();

    conditionLock->Release();
    /* 论文中提到的1处，这是一个关键点，许多语义错误在这里产生，为了保证语义正确加入semSignallerBlock */
    semWaiters->P();
    semSignallerBlock->V();

    conditionLock->Acquire();
}
void Condition::Signal(Lock* conditionLock)
{
    ASSERT(conditionLock->isHeldByCurrentThread());
    semWaitersCountMutex->P();
    if(waitersCount > 0)
    {
        waitersCount--;
        semWaiters->V();
        semSignallerBlock->P();
    }
    semWaitersCountMutex->V();
}
void Condition::Broadcast(Lock* conditionLock)
{
    ASSERT(conditionLock->isHeldByCurrentThread());
    semWaitersCountMutex->P();
    for(int i = 0; i < waitersCount; i++)
    {
        semWaiters->V();
    }
    while (waitersCount > 0)
    {
        waitersCount--;
        semSignallerBlock->P();
    }
    semWaitersCountMutex->V();
}
