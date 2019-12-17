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

Lock::~Lock() {}
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
    queue = new List;
    mutex = new Semaphore(debugName, 1);
}
Condition::~Condition() { }
void Condition::Wait(Lock* conditionLock)
{
    ASSERT(conditionLock->isHeldByCurrentThread());

    // releasing the lock and going to sleep are
    // *atomic* in Wait()

    // 这里用信号量来实现互斥，保证这一段的原子性（注释要求lock-sleep一整段是原子操作）
    // 选择主动维护队列，因为这里如果只使用信号量实现，那么还要维护等待计数
    // 而等待计数还需要额外的保护，这会造成额外的上下文切换
    // 根据 https://www.microsoft.com/en-us/research/wp-content/uploads/2004/12/ImplementingCVs.pdf 的说法
    // 只用信号量实现条件变量可以得到正确语义的结果，但是性能不行

    mutex->P();
    conditionLock->Release();
    queue->Append((void*) currentThread);
    currentThread->Sleep();
    mutex->V();

    conditionLock->Acquire();
}
void Condition::Signal(Lock* conditionLock)
{
    ASSERT(conditionLock->isHeldByCurrentThread());
    Thread* t = (Thread*) queue->Remove();
    if(t != NULL)
    {
        scheduler->ReadyToRun(t);
    }
}
void Condition::Broadcast(Lock* conditionLock)
{
    ASSERT(conditionLock->isHeldByCurrentThread());
    Thread* t;
    while ((t = (Thread*) queue->Remove()) != NULL)
    {
        scheduler->ReadyToRun(t);
    }
}
