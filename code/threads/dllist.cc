#include "dllist.h"

extern void ThreadSwitchErrnum(int expected);

DLLElement::DLLElement(void* itemPtr, int sortKey)
    : next(NULL), prev(NULL), key(sortKey), item(itemPtr) {}

DLList::DLList()
    : first(NULL), last(NULL)
    , size(0)
{}

DLList::~DLList()
{
    DLLElement* p;
    while (first != NULL)
    {
        p = first;
        first = first->next;
        ::operator delete(p->item);
        delete p;
    }
}

void DLList::Prepend(void* item)
{
    if (IsEmpty())
    {
        DLLElement* new_item = new DLLElement(item, DEFAULT_MIN_KEY);
        first = last = new_item;
    }
    else
    {
        DLLElement* new_item = new DLLElement(item, first->key - 1);
        new_item->next = first;
        first->prev = new_item;
        first = new_item;
    }
    size++;
}

void DLList::Append(void* item)
{
    if (IsEmpty())
    {
        DLLElement* new_item = new DLLElement(item, DEFAULT_MIN_KEY);
        first = last = new_item;
    }
    else
    {
        DLLElement* new_item = new DLLElement(item, last->key + 1);
        new_item->prev = last;
        last->next = new_item;
        last = new_item;
    }
    size++;
}

CHECK_RESULT
void* DLList::Remove(int* keyPtr)
{
    if (IsEmpty())
    {
        return NULL;
    }
    else
    {
        void* ret = first->item;
        if (keyPtr != NULL)
        {
            *keyPtr = first->key;
        }
        DLLElement* removed = first;
        first = first->next;
        if (first == NULL)
        {
            last = NULL;
        }
        else
        {
            first->prev = NULL;
        }
        delete removed;
        size--;
        return ret;
    }
}

bool DLList::IsEmpty()
{
    return first == NULL;
}

void DLList::SortedInsert(void* item, int sortKey)
{
    if (IsEmpty())
    {
        DLLElement* temp = new DLLElement(item, sortKey);
        first = last = temp;
    }
    else
    {
        DLLElement* p = first;
        for (; p != NULL && sortKey >= p->key; p = p->next)
            ;
        ThreadSwitchErrnum(4);
//        ThreadSwitchErrnum(5);
        // ... - p->prev - *temp* - p - p->next - ...
        // in - p -
        DLLElement* temp = new DLLElement(item, sortKey);
        if (p != NULL)
        {
            temp->next = p;
            temp->prev = p->prev;
            if (p != first)
            {
                p->prev->next = temp;
                p->prev = temp;
            }
            else
                // when sortKey < first->key *at start*, first changes
                // - 2 -
                // - 1 - 2 -
            {
                first->prev = temp;
                first = temp;
            }
        }
        // - last - *temp* - NULL
        else
        {
            last->next = temp;
            temp->prev = last;
            last = temp;
        }
    }
    size++;
}

CHECK_RESULT
void* DLList::SortedRemove(int sortKey)
{
    if (IsEmpty())
    {
        return NULL;
    }
    else
    {
        DLLElement* p = first;
        for (; p != NULL && sortKey != p->key; p = p->next)
            ;
        ThreadSwitchErrnum(5);
        if (p == NULL)
        {
            return NULL;
        }
        else
        {
            if (p == first && p == last)
            {
                first = last = NULL;
            }
            else if (p == first)
            {
                first = first->next;
                first->prev = NULL;
            }
            else if (p == last)
            {
                last = last->prev;
                last->next = NULL;
            }
            else
            {
                p->next->prev = p->prev;
                p->prev->next = p->next;
            }
            void* ret = p->item;
            delete p;
            size--;
            return ret;
        }
    }
}

const DLLElement* DLList::First() const
{
    return first;
}

const DLLElement* DLList::Last() const
{
    return last;
}

int DLList::Size() const
{
    return size;
}

SynchDLList::SynchDLList()
{
    list = new DLList();
    lock = new Lock("Dllist lock");
    listEmpty = new Condition("Dllist empty cond");
}

SynchDLList::~SynchDLList()
{
    delete list;
    delete lock;
    delete listEmpty;
}

void SynchDLList::Prepend(void *item)
{
    lock->Acquire();
    list->Prepend(item);
    listEmpty->Signal(lock);
    lock->Release();
}

void SynchDLList::Append(void *item)
{
    lock->Acquire();
    list->Append(item);
    listEmpty->Signal(lock);
    lock->Release();
}

CHECK_RESULT
void* SynchDLList::Remove(int *keyPtr)
{
    lock->Acquire();			// enforce mutual exclusion
    while (list->IsEmpty())
        listEmpty->Wait(lock);		// wait until list isn't empty
    void *item = list->Remove(keyPtr);
    ASSERT(item != NULL);
    lock->Release();
    return item;
}

bool SynchDLList::IsEmpty()
{
    return list->IsEmpty();
}

void SynchDLList::SortedInsert(void* item, int sortKey)
{
    lock->Acquire();
    list->SortedInsert(item, sortKey);
    listEmpty->Signal(lock);
    lock->Release();
}

CHECK_RESULT
void* SynchDLList::SortedRemove(int sortKey)
{
    lock->Acquire();
    while (list->IsEmpty())
    {
        listEmpty->Wait(lock);
    }
    void *item = list->SortedRemove(sortKey);
    lock->Release();
    return item;
}

//! 测试专用 自行 LockSelf 上锁
const DLLElement* SynchDLList::First() const
{
    return list->First();
}

//! 测试专用 自行 LockSelf 上锁
const DLLElement* SynchDLList::Last() const
{
    return list->Last();
}

int SynchDLList::Size() const
{
    return list->Size();
}
