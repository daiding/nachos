#pragma once
#include "synch.h"


class BoundedBuffer {
public:
    // create a bounded buffer with a limit of 'maxsize' bytes
    BoundedBuffer(int maxsize);
    ~BoundedBuffer();

    // read 'size' bytes from the bounded buffer, storing into 'data'.
    // ('size' may be greater than 'maxsize')
    void Read(void *data, int size);

    // write 'size' bytes from 'data' into the bounded buffer.
    // ('size' may be greater than 'maxsize')
    void Write(void *data, int size);

    int Size();

private:
    int bufferSize;
    int writePos;	//写入位置
    int readPos;	//读出位置
    unsigned char* buffer;
    Semaphore* s;	//实现互斥
    Semaphore* n; //缓冲区中数目
    Semaphore* e; //空闲空间数目
};

