#include "Elevator.h"
#include "system.h"
Elevator::Elevator(char *debugName, int numFloors, int myID)
{
    name = debugName;
    exitComplete = new EventBarrier[numFloors];
    currentfloor = 0; // 假定电梯起初都在0楼
    occupancy = 0;
    occupancyLock = new Lock("occupancy mutex");
    elevatorID = myID;
    request = new bool[numFloors];
    floorsNum = numFloors;
    direction = UP;
    doorState = CLOSE;
    nextFloor = 0;
    requestNum = 0;
    requestComplete = new Condition("requestComplete");
    requestNumLock = new Lock("requestNumLock");
    building = NULL; // 稍后绑定
}
Elevator::~Elevator()
{
    delete[] exitComplete;
    delete occupancyLock;
    delete[] request;
    delete requestComplete;
    delete requestNumLock;
}
void Elevator::Exit()
{
    // Rider退出时，减少occupancy并通知已退出
    occupancyLock->Acquire();
    occupancy--;
    occupancyLock->Release();
    exitComplete[currentfloor].Complete();
}
int Elevator::GetExitWaiters(int floor)
{
    return exitComplete[floor].Waiters();
}

void Elevator::NotifyBuildingEnterComplete()
{
    // 通知building，该线程的入电梯操作已完成，无论成功与否
    if (direction == UP)
    {
        building->ElevatorUpEnter(currentfloor);
    }
    else
    {
        building->ElevatorDownEnter(currentfloor);
    }
}

bool Elevator::Enter()
{
    occupancyLock->Acquire();
    if (occupancy == ELEVATOR_MAX_RIDERS)
    {
        occupancyLock->Release();
        //无论是否成功上电梯都要做出响应
        NotifyBuildingEnterComplete();
        return false;
    }
    else
    {
        occupancy++;
        occupancyLock->Release();
        requestNumLock->Acquire();
        requestNum++;
        requestNumLock->Release();
        //无论是否成功上电梯都要做出响应
        NotifyBuildingEnterComplete();
        return true;
    }
}

void Elevator::RequestFloor(int floor)
{
    requestNumLock->Acquire();
    request[floor] = true;
    requestNum--;
    // 最后一个按楼层按钮的乘客负责把电梯控制线程唤醒
    if (requestNum == 0)
    {
        DEBUG('t', "Request complete! Elevator %d will be waked up.\n", elevatorID);
        requestComplete->Signal(requestNumLock);
    }
    requestNumLock->Release();
    // 等电梯到达指定楼层后开门时通知
    exitComplete[floor].Wait();
}

void Elevator::NotifyBuildingOpenDoors()
{
    if (direction == UP)
    {
        building->ElevatorUpCome(currentfloor, elevatorID);
    }
    else // direction == DOWN
    {
        building->ElevatorDownCome(currentfloor, elevatorID);
    }
}
void Elevator::OpenDoors()
{
    DEBUG('t', "Elevator %d Open Door in Floor %d\n", elevatorID, currentfloor);
    doorState = OPEN;
    // 让乘客下电梯
    exitComplete[currentfloor].Signal();
    request[currentfloor] = false;
    //DEBUG('t', "%d\n",exitComplete[currentfloor].Waiters());
    ASSERT(exitComplete[currentfloor].Waiters() == 0);

    // 告知电梯到达
    NotifyBuildingOpenDoors();
}

void Elevator::CloseDoors()
{
    requestNumLock->Acquire();
    // 关门的时候如果发现还有乘客还没按按钮，不能关门（发请求）
    while (requestNum != 0)
    {
        DEBUG('t', "Elevator %d will go to sleep because there are riders need to request floor\n", elevatorID);
        requestComplete->Wait(requestNumLock);
    }
    ASSERT(requestNum == 0);
    requestNumLock->Release();
    DEBUG('t', "Elevator %d Close Door in Floor %d\n", elevatorID, currentfloor);
    doorState = CLOSE;
}
void Elevator::VisitFloor(int floor)
{
    DEBUG('t', "Elevator %d Visit Floor %d\n", elevatorID, floor);
    // 模拟上楼耗时
    alarmer->Pause(ELEVATOR_TIMERTICK_PER_FLOOR * TimerTicks);
    currentfloor = floor;
}
void Elevator::Run()
{
    while (true)
    {
        if (direction == UP)
        {
            DEBUG('t', "Elevator %d now in floor %d and direction is up\n", elevatorID, currentfloor);
        }
        else
        {
            DEBUG('t', "Elevator %d now in floor %d and direction is down\n", elevatorID, currentfloor);
        }
        building->ElevatorControl(this); //获取电梯要访问的下一层和方向
        if (direction == UP)
        {
            DEBUG('t', "Elevator %d will visit floor %d and direction is up\n", elevatorID, nextFloor);
        }
        else
        {
            DEBUG('t', "Elevator %d will visit floor %d and direction is down\n", elevatorID, nextFloor);
        }
        printf("[Elevator %d][Floor %d] Going to Floor %d\n", elevatorID, currentfloor, nextFloor);

        VisitFloor(nextFloor);
        ASSERT(currentfloor == nextFloor);
        printf("[Elevator %d][Floor %d] Arrived. Opening door\n", elevatorID, currentfloor);

        OpenDoors();
        CloseDoors();

        printf("[Elevator %d][Floor %d] Closed door. Occupancy: %d \n", elevatorID, currentfloor, occupancy);
    }
}

void Elevator::SetDirection(ElevatorState dir)
{
    direction = dir;
}

void Elevator::NextFloor(int floor)
{
    nextFloor = floor;
}

Elevator::ElevatorState Elevator::GetDirection()
{
    return direction;
}

Elevator::DoorState Elevator::GetDoorState()
{
    return doorState;
}

int Elevator::GetOccupancy()
{
    return occupancy;
}

int Elevator::GetCurrentFloor()
{
    return currentfloor;
}

bool Elevator::GetRequest(int floor)
{
    return request[floor];
}

bool Elevator::Full()
{
    return occupancy == ELEVATOR_MAX_RIDERS;
}

int Elevator::GetID()
{
    return elevatorID;
}

void Elevator::SetBuilding(Building *b)
{
    building = b;
}

Building::Building(char *debugname, int numFloors, int numElevators)
{
    name = debugname;
    haveWaiter = new Condition("haveWaiter");
    floorsNum = numFloors;
    elevatorsNum = numElevators;
    calledUp = new bool[numFloors];
    calledDown = new bool[numFloors];
    elevatorUpCome = new EventBarrier[numFloors];
    elevatorUpID = new int[numFloors];
    elevatorDownID = new int[numFloors];
    elevatorDownCome = new EventBarrier[numFloors];
    conditionLock = new Lock("conditionLock");
    upIDLock = new Lock("upIDLock");
    downIDLock = new Lock("downIDLock");
    calledUpLock = new Lock("calledUpLock");
    calledDownLock = new Lock("calledDownLock");
    // 因为C++的new expression只能调default constructor
    // 这里使用placement new和operator new来进行分配
    // 以在指定内存调用指定构造函数，同时完成大楼绑定
    elevator = (Elevator *)::operator new[](sizeof(Elevator) * numElevators);
    for (int i = 0; i < numElevators; i++)
    {
        new (&elevator[i]) Elevator("BuildingElevator", numFloors, i);
        elevator[i].SetBuilding(this);
    }
}

Building::~Building()
{
    delete[] calledUp;
    delete[] calledDown;
    delete[] elevatorUpID;
    delete[] elevatorDownID;
    delete upIDLock;
    delete downIDLock;
    delete[] elevatorUpCome;
    delete[] elevatorDownCome;
    delete conditionLock;
    delete haveWaiter;
    // 相应地，需要手动调用destructor
    for (int i = 0; i < elevatorsNum; i++)
    {
        elevator[i].~Elevator();
    }
    ::operator delete[](elevator);
}

void Building::CallUp(int fromFloor)
{
    // 这一楼有乘客叫电梯，通知调度线程
    calledUpLock->Acquire();
    calledUp[fromFloor] = true;
    calledUpLock->Release();
    conditionLock->Acquire();
    haveWaiter->Broadcast(conditionLock); //通知电梯有乘客到来
    conditionLock->Release();
}

void Building::CallDown(int fromFloor)
{
    // 这一楼有乘客叫电梯，通知调度线程
    calledDownLock->Acquire();
    calledDown[fromFloor] = true;
    calledDownLock->Release();
    conditionLock->Acquire();
    haveWaiter->Broadcast(conditionLock);
    conditionLock->Release();
}

Elevator *Building::AwaitUp(int fromFloor)
{
    elevatorUpCome[fromFloor].Wait(); //等待相应方向的电梯到达
    // 见ElevatorUpCome，这里加锁了所以不会出现竞争，返回的一定是加锁的最先到达电梯
    return elevator + elevatorUpID[fromFloor];
}

Elevator *Building::AwaitDown(int fromFloor)
{
    elevatorDownCome[fromFloor].Wait();
    // 见ElevatorDownCome，这里加锁了所以不会出现竞争，返回的一定是加锁的最先到达电梯
    return elevator + elevatorDownID[fromFloor];
}

void Building::ElevatorUpCome(int floor, int elevatorID)
{
    calledUpLock->Acquire();
    calledUp[floor] = false;
    calledUpLock->Release();
    upIDLock->Acquire();
    elevatorUpID[floor] = elevatorID;
    elevatorUpCome[floor].Signal();
    //ASSERT(elevator[elevatorID].Full()||elevatorUpCome[floor].Waiters()==0);
    upIDLock->Release();
}
void Building::ElevatorDownCome(int floor, int elevatorID)
{
    calledDownLock->Acquire();
    calledDown[floor] = false;
    calledDownLock->Release();
    downIDLock->Acquire();
    elevatorDownID[floor] = elevatorID;
    elevatorDownCome[floor].Signal();
    //ASSERT(elevator[elevatorID].Full()||elevatorDownCome[floor].Waiters()==0);
    downIDLock->Release();
}

void Building::ElevatorUpEnter(int floor)
{
    elevatorUpCome[floor].Complete();
}

void Building::ElevatorDownEnter(int floor)
{
    elevatorDownCome[floor].Complete();
}
// 电梯调度策略（以当前运行方向为上为例）：
// 如果电梯上无乘客且当前楼层以上的楼层无等待电梯向上的乘客，则电梯改变方向，并且访问有需要下楼乘客的最高层；
// 若电梯内有乘客或者所在楼层以上的楼层有需要上楼的乘客，则访问 min(存在要上楼乘客的楼层（要求电梯未满员）,乘客要下电梯的楼层)
// 若没有任何乘客，则等待乘客召唤
// 该函数由Elevator方调用
void Building::ElevatorControl(Elevator *elev)
{
    int nextFloor = -1;
    Elevator::ElevatorState direct = elev->GetDirection();
    do
    {
        if (elev->GetDirection() == Elevator::UP)
        {
            for (int i = elev->GetCurrentFloor(); i < floorsNum; i++)
            {
                bool flag;
                calledUpLock->Acquire();
                flag = calledUp[i] || elevatorUpCome[i].Waiters() > 0;
                calledUpLock->Release();
                if ((flag && !elev->Full()) || elev->GetExitWaiters(i) > 0 || elev->GetRequest(i))
                {
                    nextFloor = i;
                    break;
                }
            }

            if (nextFloor == -1)
            {
                for (int i = floorsNum - 1; i >= 0; i--)
                {
                    bool downflag, upflag;
                    calledDownLock->Acquire();
                    downflag = calledDown[i] || elevatorDownCome[i].Waiters() > 0;
                    calledDownLock->Release();
                    calledUpLock->Acquire();
                    upflag = calledUp[i] || elevatorUpCome[i].Waiters() > 0;
                    calledUpLock->Release();
                    if (downflag || upflag)
                    {
                        if (downflag)
                            direct = Elevator::DOWN;
                        nextFloor = i;
                        break;
                    }
                }
            }
        }
        else
        {
            for (int i = elev->GetCurrentFloor(); i >= 0; i--)
            {
                bool flag;
                calledDownLock->Acquire();
                flag = calledDown[i] || elevatorDownCome[i].Waiters() > 0;
                calledDownLock->Release();
                if ((flag && !elev->Full()) || elev->GetExitWaiters(i) > 0 || elev->GetRequest(i))
                {
                    nextFloor = i;
                    break;
                }
            }
            if (nextFloor == -1)
            {
                for (int i = 0; i < floorsNum; i++)
                {
                    bool downflag, upflag;
                    calledDownLock->Acquire();
                    downflag = calledDown[i] || elevatorDownCome[i].Waiters() > 0;
                    calledDownLock->Release();
                    calledUpLock->Acquire();
                    upflag = calledUp[i] || elevatorUpCome[i].Waiters() > 0;
                    calledUpLock->Release();
                    if (downflag || upflag)
                    {
                        nextFloor = i;
                        if (upflag)
                            direct = Elevator::UP;
                        break;
                    }
                }
            }
        }
        if (nextFloor == -1) //表明整栋楼都没有乘客
        {
            DEBUG('t', "Elevator %d will go to sleep because of no waiters\n", elev->GetID());
            conditionLock->Acquire();
            haveWaiter->Wait(conditionLock); //等待乘客出现
            conditionLock->Release();
        }
    } while (nextFloor == -1);
    elev->SetDirection(direct);
    elev->NextFloor(nextFloor);
}

Elevator *Building::GetElevator()
{
    return elevator;
}
