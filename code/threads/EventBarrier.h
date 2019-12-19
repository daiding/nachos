#include "synch.h"
class EventBarrier
{
public:
    enum SignalState
    {
        SIGNALED = 1,
        UNSIGNALED = 0
    };
    EventBarrier();
    ~EventBarrier();

    void Wait();   //Wait until the event is signaled.Return immediately if already in the signaled state.
    void Signal(); //Signal the event and block until all thread that wait for this event have responded.
    //The EventBarrier reverts to the unsignaled state when Signal() returns.
    void Complete(); //Indicate that the calling thread has finished responding to a signaled event ,and
    //block until all other threads that wait for this event have also responded.
    int Waiters(); //Return a count of threads that are waiting for the event or that have not yet responded to it

private:
    int count;
    SignalState state;
    char *debugName;
    Condition *event;
    Condition *complete;
    Lock *lock;
};
