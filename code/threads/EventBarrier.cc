#include "EventBarrier.h"
#include "system.h"
#include "thread.h"

EventBarrier::EventBarrier()
{
    count = 0;
    state = UNSIGNALED;
    debugName = "EventBarrier";
    event = new Condition("EventSignal");
    complete = new Condition("Completes");
    lock = new Lock("EventBarrierLock");
}

EventBarrier::~EventBarrier()
{
    delete lock;
    delete complete;
    delete event;
}

void EventBarrier::Wait()
{
    lock->Acquire();
    count++;
    if (state == SIGNALED)
    {
        lock->Release();
        return;
    }
    event->Wait(lock);
    lock->Release();
}

void EventBarrier::Signal()
{
    // 没有等待者的时候，Signal()该怎么做，实验要求并没有规定。已知三种实现方式
    // 1. 发信号的线程马上离开函数，屏障解除，屏障恢复为UNSIGNALED
    // 2. 发信号的线程停在函数里，屏障保持SIGNALED，直到有线程Wait()（直接跳过）并Complete()，然后屏障解除
    //    这样的话，能有多少个线程收到这个事件信号完全取决于它们是否在发信号的线程离开屏障之前到达屏障
    // 3. 构造函数提供类似participantCount的东西，例如pthread的barrier / C# 的Barrier / Java的CyclicBarrier都在初始化的时候给了参与线程数量
    //
    // 考虑电梯问题，本代码使用1 
    //
    // 打开随机顺序切换(-rs <num>)，threadtest.cc的EventWaiterThread的被注释代码出现这样的情形：
    // 发信号线程的先走了，后面线程没收到信号，停在栅栏等待下一个事件信号
    // 虽然这也是对的，不过总感觉很奇怪，不事先约定一下到多少个真的好吗……
    // 
    // 考虑到这个不定项 【事件】 栅栏，好像也有点道理
    //
    // 虽然很多库的barrier没有严格的区分signaller / waiter，而是统一处理，而pthread的barrier更是严格要求参与数不能为0
    // （但这个EventBarrier和pthread的以及其他的不同，如果把Signaller算入，参与数至少为1了）
    // https://linux.die.net/man/3/pthread_barrier_init
    // The count argument specifies the number of threads that must call pthread_barrier_wait() before any of them successfully return from the call. The value specified by count must be greater than zero.
    ASSERT(state != SIGNALED);
    lock->Acquire();
    state = SIGNALED;
    event->Broadcast(lock);
    if(count > 0) // 必须等待这一波信号的所有人走完
    {
        complete->Wait(lock);
        ASSERT(state == UNSIGNALED); // 见EventBarrier::Complete()
    }
    else
    {
        ASSERT(count == 0);
        state = UNSIGNALED;
    }
    lock->Release();
}

void EventBarrier::Complete()
{
    lock->Acquire();
    count--;
    if (count == 0)
    {
        // Ensure thread will wait next Signal when it Wait immediately after complete
        // 如果这里不UNSIGNAL，那么这个线程在这个函数结束后立刻进行Wait（中间不抢占）会马上结束
        // 不满足上述要求
        state = UNSIGNALED; 
        complete->Broadcast(lock);
    }
    else
    {
        complete->Wait(lock);
    }
    lock->Release();
}

int EventBarrier::Waiters()
{
    lock->Acquire();
    int ret = count;
    lock->Release();
    return ret;
}
