/*

Here are the method signatures for the Elevator and Building classes.
You should feel free to add to these classes, but do not change the
existing interfaces.

*/
#include "EventBarrier.h"
#include <new>
class Building;
class Elevator;

class Elevator
{
public:
    enum ElevatorState
    {
        UP = 1,
        DOWN = 0
    };
    enum DoorState
    {
        OPEN = 1,
        CLOSE = 0
    };
    enum
    {
        // 操作系统服务器的旧版GCC不支持static const int 在类内的直接定义…… 
        ELEVATOR_MAX_RIDERS = 20,
        ELEVATOR_TIMERTICK_PER_FLOOR = 10 
    };
    Elevator(char *debugName, int numFloors, int myID);
    ~Elevator();
    char *getName()
    {
        return name;
    }

    // elevator control interface: called by Elevator thread
    void OpenDoors();           //   signal exiters and enterers to action
    void CloseDoors();          //   after exiters are out and enterers are in
    void VisitFloor(int floor); //   go to a particular floor

    // elevator rider interface (part 1): called by rider threads.
    bool Enter();                 //   get in
    void Exit();                  //   get out (iff destinationFloor)
    void RequestFloor(int floor); //   tell the elevator our destinationFloor

    // insert your methods here, if needed

    void Run();                        //电梯线程在此无限运行
    void SetDirection(ElevatorState dir); //改变电梯方向
    void NextFloor(int floor);         //设置电梯下一次访问楼层
    ElevatorState GetDirection();
    DoorState GetDoorState();
    int GetOccupancy();
    int GetCurrentFloor();
    bool Full(); //判断电梯是否满载
    int GetID();
    bool GetRequest(int floor);
    int GetExitWaiters(int floor);
    void SetBuilding(Building *b);
private:
    void NotifyBuildingEnterComplete();
    void NotifyBuildingOpenDoors();
private:
    char *name;
    // 事件：到达的乘客已退出电梯
    EventBarrier *exitComplete;
    // floor where currently stopped
    int currentfloor; 
    // how many riders currently onboard
    int occupancy;    
    // occupancy的互斥锁
    Lock *occupancyLock;       
    // 电梯编号
    int elevatorID;
    // 数组：某楼层有请求（供Building使用）
    bool *request;
    // 楼层数
    int floorsNum;
    // 电梯方向
    ElevatorState direction;
    // 门状态
    DoorState doorState;
    // 下一次访问的楼层
    int nextFloor;
    // 请求等待数
    int requestNum;
    // 请求已完成（指示可以关门了）
    Condition *requestComplete;
    // requestComplete锁
    Lock *requestNumLock;
    // 关联的大楼，在Building构造函数中设置
    // 在Run()执行期回调building，让building决定Elevator的行为
    Building *building; 
};

class Building
{
public:
    Building(char *debugname, int numFloors, int numElevators);
    ~Building();
    char *getName()
    {
        return name;
    }

    // elevator rider interface (part 2): called by rider threads
    void CallUp(int fromFloor);         //   signal an elevator we want to go up
    void CallDown(int fromFloor);       //   ... down
    Elevator *AwaitUp(int fromFloor);   // wait for elevator arrival & going up
    Elevator *AwaitDown(int fromFloor); // ... down

    Elevator *GetElevator();
    // 向上的电梯来了
    void ElevatorUpCome(int floor, int elevatorID); 
    // 向下的电梯来了
    void ElevatorDownCome(int floor, int elevatorID);
    // 向上的乘客已试图进入电梯
    void ElevatorUpEnter(int floor);
    // 向下的乘客已试图进入电梯 
    void ElevatorDownEnter(int floor);
    // 控制电梯，可以改变电梯的方向以及下一次访问的楼层
    void ElevatorControl(Elevator *elev); 
private:
    char *name;
    Elevator *elevator; // the elevators in the building (array)

    // insert your data structures here, if needed
    // 电梯数量
    int elevatorsNum;
    // 楼层数量
    int floorsNum;
    // 某楼层是否有乘客召唤向上
    bool *calledUp; 
    // 某楼层是否有乘客向下
    bool *calledDown;
    // calledUp锁
    Lock *calledUpLock;
    // calledDown锁
    Lock *calledDownLock;
    // 向上到达指定楼层的电梯ID
    int *elevatorUpID; 
    // 向下到达指定楼层的电梯ID
    int *elevatorDownID;
    // elevatorUpID锁
    Lock *upIDLock;
    // elevatorDownID锁
    Lock *downIDLock;
    // 等待上楼电梯到来
    EventBarrier *elevatorUpCome; 
    // 等待下楼电梯到来
    EventBarrier *elevatorDownCome;
    // haveWaiter条件锁
    Lock *conditionLock;
    // 条件变量：有等待电梯的人
    Condition *haveWaiter; 
};
