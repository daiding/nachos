#pragma once
#include<cstddef>
#include"synch.h"

// 这个宏用来检查返回值是否被使用 放在函数**原型**头
// 注意，不应只放在定义头，否则对于include头文件的其他编译单元失效
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define CHECK_RESULT __attribute__ ((warn_unused_result))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#define CHECK_RESULT _Check_return_
#else
#define CHECK_RESULT
#endif

// 禁用拷贝构造函数和拷贝赋值函数
struct NonCopyable
{
private:
    NonCopyable(const NonCopyable&) {};
    NonCopyable& operator=(const NonCopyable&) {
        return *this;
    };
public:
    NonCopyable() {};
};

// DLList类的元素
// 注意 : 这个类的析构函数**有意**不加定义，并且只被认为是一个可平凡析构的东西，资源需自行管理
//          不要忘记释放 DLLElement::item
class DLLElement {
public:
    DLLElement(void* itemPtr, int sortKey); // initialize a list element

    DLLElement* next; // next element on list
    // NULL if this is the last
    DLLElement* prev; // previous element on list
    // NULL if this is the first
    int key;	// priority, for a sorted list
    void* item; // pointer to item on the list

    // I REALLY DON'T KNOW why the course pdf use void* instead of template<typename T> T* item
    // Because of type erasure, item cannot point to any class with non-trivial destructor
    // Otherwise,
    // delete item -> undefined behaviour
    // free(item)  -> undefined behaviour
    // SO... USE POD type or trivially destructible type ONLY
};


// Doublely Linked SortedList
// 继承了NonCopyable的原因：Nachos的内存分配器很诡异，非POD对象成员变量放栈上似乎会报一些莫名其妙的运行时错误，加上PDF没有定义拷贝赋值函数之类的，干脆把它禁了好了
class DLList : NonCopyable {
public:
    enum {DEFAULT_MIN_KEY = 0}; // Old version of G++ cannot compile `static const int CONST = 0;`
    DLList(); // initialize the list
    ~DLList(); // de-allocate the list
    void Prepend(void* item);  // add to head of list (set key = min_key-1)
    void Append(void* item);   // add to tail of list (set key = max_key+1)
    CHECK_RESULT void* Remove(int* keyPtr); // remove from head of list
    // set *keyPtr to key of the removed item
    // return item (or NULL if list is empty)
    bool IsEmpty(); // return true if list has *NO* elements

    // routines to put/get items on/off list in order (sorted by key)
    void SortedInsert(void* item, int sortKey);
    CHECK_RESULT void* SortedRemove(int sortKey); // remove first item with key==sortKey
    // return NULL if no such item exists

    const DLLElement* First() const;
    const DLLElement* Last() const;

    int Size() const;

private:
    DLLElement* first; // head of the list, NULL if empty
    DLLElement* last; // last element of the list, NULL if empty

    int size;
};

class SynchDLList : NonCopyable {
public:
    enum {DEFAULT_MIN_KEY = 0}; // Old version of G++ cannot compile `static const int CONST = 0;`
    SynchDLList(); // initialize the list
    ~SynchDLList(); // de-allocate the list

    void Prepend(void* item);  // add to head of list (set key = min_key-1)
    void Append(void* item);   // add to tail of list (set key = max_key+1)
    CHECK_RESULT void* Remove(int* keyPtr); // remove from head of list
    // set *keyPtr to key of the removed item
    // return item (or NULL if list is empty)
    bool IsEmpty(); // return true if list has *NO* elements

    // routines to put/get items on/off list in order (sorted by key)
    void SortedInsert(void* item, int sortKey);
    CHECK_RESULT void* SortedRemove(int sortKey); // remove first item with key==sortKey
    // return NULL if no such item exists

    // 测试专用
    void LockSelf() {
        lock->Acquire();
    }
    void UnlockSelf() {
        lock->Release();
    }
    const DLLElement* First() const;
    const DLLElement* Last() const;
    int Size() const;

private:
    DLList *list;
    Lock *lock;
    Condition *listEmpty;
};