#include "syscall.h"

int main()
{
    int a = 1;
    int b = 3;
    int c = a + b;
    Exec("../test/halt",0,0,1);
    return 0;
}
