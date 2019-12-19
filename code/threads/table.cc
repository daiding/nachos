#include "table.h"
extern void ThreadSwitchErrnum(int expected);

Table::Table(int size)
{
    tableLock = new Lock("tablelock");
    tableSize = size;
    table = reinterpret_cast<void**>(new char*[size]);
    for (int i = 0; i < size; i++)
    {
        table[i] = NULL;
    }
}

Table::~Table()
{
    delete tableLock;
    delete[] (char*)table;
}

int Table::Alloc(void * object)
{
    tableLock->Acquire();
    int ret = -1;
    for (int i = 0; i < tableSize; i++)
    {
        if (table[i] == NULL)
        {
            ThreadSwitchErrnum(4);
            table[i] = object;
            ret = i;
            break;
        }
    }
    tableLock->Release();
    return ret;
}

void * Table::Get(int index)
{
    ASSERT(index <= tableSize - 1)
    tableLock->Acquire();
    void * ret = table[index];
    tableLock->Release();
    return ret;
}

void Table::Release(int index)
{
    ASSERT(index >= 0 || index < tableSize);
    tableLock->Acquire();
    ThreadSwitchErrnum(5);
    table[index] = NULL;
    tableLock->Release();
}

int Table::Size()
{
    // tableSize is read-only in fact
    return tableSize;
}