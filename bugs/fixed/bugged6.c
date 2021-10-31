/******************************************************************************
* ЗАДАНИЕ: bugged6.c
* ОПИСАНИЕ:
*   Множественные ошибки компиляции
******************************************************************************/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define VECLEN 100

float a[VECLEN], b[VECLEN];
float sum; // must be in global scope

float dotprod()
{
    int i, tid;
    // float sum;

    tid = omp_get_thread_num();
    #pragma omp for reduction(+:sum)
    for (i = 0; i < VECLEN; i++)
    {
        sum = sum + (a[i] * b[i]);
        printf("  tid= %d i=%d\n", tid, i);
    }
}
 
int main (int argc, char *argv[])
{
    int i;
    

    for (i = 0; i < VECLEN; i++)
        a[i] = b[i] = 1.0 * i;
    sum = 0.0;

    #pragma omp parallel shared(sum)
    {
        dotprod();
    } // omp parallel

    printf("Sum = %f\n",sum);
}


