/******************************************************************************
* ЗАДАНИЕ: bugged1.c
* ОПИСАНИЕ:
*   Данная программа демонстрирует использование конструкции 'parallel for'.
*   Однако, данный код вызывает ошибки компиляции.
******************************************************************************/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

// #include <assert.h>

#define N           50
#define CHUNKSIZE   5

int main(int argc, char **argv)
{
    int i, chunk, tid;
    float a[N], b[N], c[N];

    for (i = 0; i < N; i++)
        a[i] = b[i] = i * 1.0;
    chunk = CHUNKSIZE;

    #pragma omp parallel shared(a,b,c,chunk) private(i, tid)
    {
        // tid = omp_get_thread_num();
        #pragma omp parallel for schedule(static, chunk)
        for (i = 0; i < N; i++)
        {
            // assert(tid == omp_get_thread_num());
            tid = omp_get_thread_num();
            c[i] = a[i] + b[i];
            printf("tid= %d i= %d c[i]= %f\n", tid, i, c[i]);
        }
    } // omp parallel
}

/*
 * add omp parallel
 * exclude tide after omp parallel for
 */ 
