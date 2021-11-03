/******************************************************************************
* ЗАДАНИЕ: bugged2.c
* ОПИСАНИЕ:
*   Еще одна программа на OpenMP с багом. 
******************************************************************************/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    double total = 0.0;

    #pragma omp parallel shared(total)// private(total) //by default
    {
        int tid = omp_get_thread_num();
        if (tid == 0)
        {
            int nthreads = omp_get_num_threads();
            printf("Number of threads = %d\n", nthreads);
        }
        printf("Thread %d is starting...\n", omp_get_thread_num());

        #pragma omp barrier

        #pragma omp for schedule(dynamic, 10) reduction(+: total)
        for (int i = 0; i < 1000000; i++) {
            total = total + i*1.0;
        }

        // printf ("Thread %d is done! Total= %e\n", omp_get_thread_num(), total);
    }

    printf ("Thread %d is done! Total= %le\n", omp_get_thread_num(), total);
}

/*
 * tid is replaced to omp_get_thread_num()
 * added reduction
 */
