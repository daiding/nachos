#include "table-driver.h"
#include "system.h"
#include <cstdio>

void TableInsert(Table * table, int size, int which)
{
    ASSERT(table != NULL);
    for(int i = 0; i < size; i++)
    {
        char * element = new char('a' + i % 26);
        int index = table->Alloc(reinterpret_cast<void*>(element));
        if(index == -1)
        {
            printf("[Thread %d] Table is full, wait a moment and try again\n", which);
            delete element;
            i--;
            currentThread->Yield();
            continue;
        }
        else
        {
            ASSERT(*element == *(char*)table->Get(index));
            printf("[Thread %d] Insert '%c' to slot %d\n", which, *element, index);
        }
    }
}

void TableRemove(Table* table, int size, int which)
{
    ASSERT(table != NULL);
    for(int i = 0; i < table->Size() && size > 0; i++)
    {
        char* element = (char*)table->Get(i);
        if(element == NULL)
        {
            printf("[Thread %d] Table slot %d is NULL\n", which, i);
        }
        else
        {
            printf("[Thread %d] Remove element %c in slot %d\n", which, *element, i);
            table->Release(i);
            size--;
            delete element;
        }
    }
}