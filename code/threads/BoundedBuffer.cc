#include "BoundedBuffer.h"
//参考书本图5.13

BoundedBuffer::BoundedBuffer(int maxsize)
{
	bufferSize = maxsize;
	buffer =reinterpret_cast<void**>( new char* [bufferSize]);
	for (int i = 0; i < maxsize; i++)
		buffer[i] = NULL;
	write = 0;
	read = 0;
	s = new Semaphore("", 1);
	n = new Semaphore("", 0);
	e = new Semaphore("", bufferSize);
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
	char* temp = (char*)data;
	//data本应为一个指针数组，此处操作不便暂时不处理
	while (size--)
	{
		n->P();		//等待缓冲区有位置
		s->P();
		/*读操作*/
		*temp++ = *((char *)buffer[read]);
		read = (read + 1) % bufferSize;
		s->V();
		e->V();		//通知缓冲区有空

	}
}

void BoundedBuffer::Write(void * data, int size)
{
	char *v = (char*)data;		//假设全为char*
	while (size--)
	{
		e->P();
		s->P();
		buffer[write] = v++;
		write = (write + 1) % bufferSize;
		s->V();
		n->V();
	}
}
