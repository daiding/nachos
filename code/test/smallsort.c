/* sort.c
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

#define NUM 20
int A[NUM];	/* size of physical memory; with code, we'll run out of space!*/

int
main()
{
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < NUM; i++) {
        A[i] = NUM - 1 - i;
    }

    /* then sort! */
    for (i = 0; i < NUM; i++) {
        for (j = 0; j < i; j++) {
            if (A[i] < A[j]) {	/* out of order -> need to swap ! */
                tmp = A[j];
                A[j] = A[i];
                A[i] = tmp;
            }
        }
    }
    for (i = 0; i < NUM; i++)
    {
        if (A[i] != i)
        {
            Exit(1);
        }
    }
    Exit(A[0]);		/* and then we're done -- should be 0! */
}
