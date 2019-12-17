#include<BoundedBuffer-driver.h>
#include<cstdio>


void BufferWrite(BoundedBuffer& buffer,int size,int which)
{
    char data[]="abcdefghijklmn";
    printf("[Thread %d] try to write %d times\n", which,size);
    buffer.Write(data,size);
    printf("[Thread %d] write end\n", which);
}

void BufferRead(BoundedBuffer& buffer,int size,int which)
{
    char* temp=new char;
    printf("[Thread %d] try to read %d times\n", which,size);
    buffer.Read(temp,size);
    printf("[Thread %d] read end\n", which);
    delete temp;
}