#pragma once
#include "list.h"
// It should be a Singleton Class
class Alarm
{
public:
    Alarm();
    ~Alarm();
    // Pause for AT LEAST (how long * TimerTicks) ticks
    void Pause(int howLong);
    // WARNING: SHOULD NOT be called by regular functions, it's intended for TimerInterruptHandler Only
    void AwakeTimeoutThreads();
    int GetWaiterCount() {
        return waiterCount;
    }
private:
    List *queue;
    int waiterCount;
};

