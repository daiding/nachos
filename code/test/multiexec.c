#include "syscall.h"

int main()
{
    int i, pid;    
    int pids[5];
    for(i=0; i<5; i++)
    {
        pid = Exec("./smallsort",0,0,1);
        if(pid == -1)
        {
            Exit(1);
        }
    }
    for(i=0;i<5;i++)
    {
        pids[i] = Exec("./smallsort",0,0,1);
        if(pids[i] == -1)
        {
            Exit(1);
        }
    }
    for(i=0;i<5;i++)
    {
        Join(pids[i]);
    }
    Exit(0);
}
