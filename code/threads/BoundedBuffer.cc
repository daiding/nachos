// 参考书本图5.13
#include "BoundedBuffer.h"

extern void ThreadSwitchErrnum(int expected);

BoundedBuffer::BoundedBuffer(int maxsize)
{
    bufferSize = maxsize;
    buffer = new unsigned char[bufferSize];
    memset(buffer, 0, bufferSize);
    writePos = 0;
    readPos = 0;
    s = new Semaphore("Mutex", 1);
    n = new Semaphore("Produced", 0);
    e = new Semaphore("Available", bufferSize);
}

BoundedBuffer::~BoundedBuffer()
{
    delete[] buffer;
    delete s;
    delete n;
    delete e;
}

void BoundedBuffer::Read(void * data, int size)
{
    unsigned char* dest = reinterpret_cast<unsigned char*>(data);
    while (size--)
    {
        n->P();		//等待缓冲区有位置
        s->P();

        ThreadSwitchErrnum(5);
        *dest++ = buffer[readPos];
        readPos = (readPos + 1) % bufferSize;

        s->V();
        e->V();		//通知缓冲区有空
    }
}

void BoundedBuffer::Write(void * data, int size)
{
    unsigned char *src = reinterpret_cast<unsigned char*>(data);
    while (size--)
    {
        e->P();
        s->P();

        ThreadSwitchErrnum(4);
        buffer[writePos] = *src++;
        writePos = (writePos + 1) % bufferSize;

        s->V();
        n->V();
    }
}

int BoundedBuffer::Size()
{
    return bufferSize;
}