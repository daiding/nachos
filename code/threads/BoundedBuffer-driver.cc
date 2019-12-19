#include"BoundedBuffer-driver.h"
#include<cstdio>
#include<cassert>

void BufferWrite(BoundedBuffer * buffer, int size, int which)
{
    char data[2] = {0};
    for(int i = 0; i < size; i++)
    {
        data[0] = 'a' + Random() % 26;
        printf("[Thread %d] Writing : %s\n", which, data);
        buffer->Write(data, 1);
    }
}

void BufferRead(BoundedBuffer * buffer, int size, int which)
{
    char data[2] = {0};
    for(int i = 0; i < size; i++)
    {
        buffer->Read(data, 1);
        printf("[Thread %d] Read : %s\n", which, data);
    }
}