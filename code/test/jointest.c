#include "syscall.h"

int main()
{
    int pid = Exec("../test/sort",0,0,1);
    int result = Join(pid);
    result++;
    return 0;
}
