#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cassert>
#include <vector>
#include <algorithm>
#include "dllist.h"
#include "system.h"

extern int errornum;
extern void ThreadSwitchErrnum(int expected);

#ifdef THREADTEST_RANDOM
// Helper func
void ThreadSwitchRandom()
{
    static bool inited = false;
    if(!inited)
    {
        srand(time(0));
        inited = true;
    }

    if(rand() % 2)
    {
        printf("[Yield] Thread random switch!\n");
        currentThread->Yield();
    }
}
#endif

void DLListRandomInsertKeys(int n, DLList* list, int threadID)
{
    assert(list != NULL);

    for (int i = 0; i < n; i++)
    {
        int r = rand() % 32767;
        list->SortedInsert(reinterpret_cast<void*>(new int(5)), r);

#ifdef DLLIST_DRIVER_OUTPUT
        printf("[Thread %d] Insert item %d : key = %d, *item = %d\n", threadID,
               i + 1, r, 5);
#endif // DLLIST_DRIVER_OUTPUT

        ThreadSwitchErrnum(2);

#ifdef THREADTEST_RANDOM
        ThreadSwitchRandom();
#endif // THREADTEST_RANDOM

    }
}

void DLListSequentialPrependKeys(int n, DLList* list, int threadID)
{
    assert(list != NULL);

    for (int i = 0; i< n; i++)
    {
        list->Prepend(reinterpret_cast<void*>(new int(i)));

#ifdef DLLIST_DRIVER_OUTPUT
        printf("[Thread %d] Prepend item %d : key = %d, *item = %d\n", threadID,
               i + 1, list->First()->key, i);
#endif // DLLIST_DRIVER_OUTPUT

        ThreadSwitchErrnum(2);

#ifdef THREADTEST_RANDOM
        ThreadSwitchRandom();
#endif // THREADTEST_RANDOM
    }
}

void DLListSequentialAppendKeys(int n, DLList* list, int threadID)
{
    assert(list != NULL);
    for (int i = 0; i < n; i++)
    {
        list->Append(reinterpret_cast<void*>(new int(i)));
#ifdef DLLIST_DRIVER_OUTPUT
        printf("[Thread %d] Append item %d : key = %d, *item = %d\n", threadID,
               i + 1, list->Last()->key, i);
#endif // DLLIST_DRIVER_OUTPUT 

        ThreadSwitchErrnum(2);

#ifdef THREADTEST_RANDOM
        ThreadSwitchRandom();
#endif // THREADTEST_RANDOM
    }
}

void DLListRemoveHeadItems(int n, DLList* list, int threadID)
{
    assert(list != NULL);

    for (int i = 0; i < n; i++)
    {
        int key;
        printf("[Thread %d] Removing head item... \n", threadID);
        int* pItem = static_cast<int*>(list->Remove(&key));

#ifdef THREADTEST_RANDOM
        ThreadSwitchRandom();
#endif // THREADTEST_RANDOM

        if (pItem == NULL)
        {
#ifdef DLLIST_DRIVER_OUTPUT
            printf("[Thread %d] Removed item %d : list is empty now\n",threadID, i + 1);
#endif // DLLIST_DRIVER_OUTPUT
            break;
        }
        else
        {
#ifdef DLLIST_DRIVER_OUTPUT
            printf("[Thread %d] Removed item %d : key = %d, *item = %d\n", threadID,
                   i + 1, key, *pItem);
#endif // DLLIST_DRIVER_OUTPUT
            delete pItem;
        }
        ThreadSwitchErrnum(3);
    }
}


void DLListRandomRemoveItems(int n, DLList* list, int threadID)
{
    assert(list != NULL);
    std::vector<int> v;
    for(const DLLElement* p = list->First(); p != NULL; p=p->next)
    {
        v.push_back(p->key);
    }
    std::random_shuffle(v.begin(), v.end());
    for (int i = 0; i < n && i < (int)v.size(); i++)
    {
        int * item = (int*)list->SortedRemove(v[i]);
#ifdef DLLIST_DRIVER_OUTPUT
        if(item != NULL)
        {
            printf("[Thread %d] Removed item : key = %d, *item = %d\n", threadID, v[i], *item);
        }
        else
        {
            printf("[Thread %d] Not found item whose key = %d\n",threadID, v[i]);
        }
#endif // DLLIST_DRIVER_OUTPUT
        delete item;
    }
}


void SynchDLListRandomInsertKeys(int n, SynchDLList* list, int threadID)
{
    assert(list != NULL);
    for (int i = 0; i < n; i++)
    {
        int r = rand() % 32767;
        list->SortedInsert(reinterpret_cast<void*>(new int(5)), r);

#ifdef DLLIST_DRIVER_OUTPUT
        printf("[Thread %d] Insert item %d : key = %d, *item = %d\n", threadID,
               i + 1, r, 5);
#endif // DLLIST_DRIVER_OUTPUT

        ThreadSwitchErrnum(2);

#ifdef THREADTEST_RANDOM
        ThreadSwitchRandom();
#endif // THREADTEST_RANDOM
    }
}

void SynchDLListSequentialPrependKeys(int n, SynchDLList* list, int threadID)
{
    assert(list != NULL);
    for (int i = 0; i< n; i++)
    {
        list->Prepend(reinterpret_cast<void*>(new int(i)));

#ifdef DLLIST_DRIVER_OUTPUT
        printf("[Thread %d] Prepend item %d : key = %d, *item = %d\n", threadID,
               i + 1, list->First()->key, i);
#endif // DLLIST_DRIVER_OUTPUT 

        ThreadSwitchErrnum(2);

#ifdef THREADTEST_RANDOM
        ThreadSwitchRandom();
#endif // THREADTEST_RANDOM
    }
}

void SynchDLListSequentialAppendKeys(int n, SynchDLList* list, int threadID)
{
    assert(list != NULL);
    for (int i = 0; i < n; i++)
    {
        list->Append(reinterpret_cast<void*>(new int(i)));

#ifdef DLLIST_DRIVER_OUTPUT
        printf("[Thread %d] Append item %d : key = %d, *item = %d\n", threadID,
               i + 1, list->Last()->key, i);
#endif // DLLIST_DRIVER_OUTPUT

        ThreadSwitchErrnum(2);

#ifdef THREADTEST_RANDOM
        ThreadSwitchRandom();
#endif // THREADTEST_RANDOM
    }
}

void SynchDLListRemoveHeadItems(int n, SynchDLList* list, int threadID)
{
    assert(list != NULL);
    for (int i = 0; i < n; i++)
    {
        int key;
        printf("[Thread %d] Removing head item... \n", threadID);
        int* pItem = static_cast<int*>(list->Remove(&key));

#ifdef THREADTEST_RANDOM
        ThreadSwitchRandom();
#endif // THREADTEST_RANDOM

        if (pItem == NULL)
        {
#ifdef DLLIST_DRIVER_OUTPUT
            printf("[Thread %d] Removed item %d : list is empty now\n",threadID, i + 1);
#endif // DLLIST_DRIVER_OUTPUT
            break;
        }
        else
        {
#ifdef DLLIST_DRIVER_OUTPUT
            printf("[Thread %d] Removed item %d : key = %d, *item = %d\n", threadID,
                   i + 1, key, *pItem);
#endif // DLLIST_DRIVER_OUTPUT
            delete pItem;
        }

        ThreadSwitchErrnum(3);
    }
}

void SynchDLListRandomRemoveItems(int n, SynchDLList* list, int threadID)
{
    assert(list != NULL);
    std::vector<int> v;
    list->LockSelf();
    for(const DLLElement* p = list->First(); p != NULL; p=p->next)
    {
        v.push_back(p->key);
    }
    list->UnlockSelf();
    std::random_shuffle(v.begin(), v.end());
    for (int i = 0; i < n && i < (int)v.size(); i++)
    {
        int * item = (int*)list->SortedRemove(v[i]);
#ifdef DLLIST_DRIVER_OUTPUT
        if(item != NULL)
        {
            printf("[Thread %d] Removed item : key = %d, *item = %d\n", threadID, v[i], *item);
        }
        else
        {
            printf("[Thread %d] Not found item whose key = %d\n",threadID, v[i]);
        }
#endif
        delete item;
    }
}
