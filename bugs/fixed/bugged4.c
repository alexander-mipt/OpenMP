/******************************************************************************
* ЗАДАНИЕ: bugged4.c
* ОПИСАНИЕ:
*   Очень простая программа с segmentation fault.
******************************************************************************/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 1048

int main (int argc, char *argv[]) 
{
    int nthreads, tid, i, j;
    double** a = (double**) calloc(N, sizeof(double));
    for (int i = 0; i < N; ++i) {
        a[i] = (double*) calloc(N, sizeof(double));
    }

    #pragma omp parallel shared(nthreads,a) private(i, j, tid)
    {
        tid = omp_get_thread_num();
        if (tid == 0) 
        {
            nthreads = omp_get_num_threads();
            printf("Number of threads = %d\n", nthreads);
        }
        printf("Thread %d starting...\n", tid);

        for (i = 0; i < N; i++)
            for (j = 0; j < N; j++)
                a[i][j] = tid + i + j;

        printf("Thread %d done. Last element= %f\n", tid, a[N-1][N-1]);
    } 
}

/*
 * there was stack overflow: need to allocate on the heap
 */
