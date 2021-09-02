#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdint.h>
#include "assert.h"

void init_omp(int threads_num) {
    // omp_set_dynamic(0);     // Explicitly disable dynamic teams
    omp_set_num_threads(10); // Use 4 threads for all consecutive parallel regions
}

void info(uint64_t max) {
    printf("threads num: %d\n", omp_get_num_threads());
    printf("count up to N]: %lu\n", max);
}

int main(int argc, char* argv[]) {
    assert(argc <= 3);

    uint64_t max_num = 20;
    uint64_t sum = 0;

    if (argc > 1) {
        int threads_num = atoi(argv[1]);
        assert(threads_num > 0);
        omp_set_num_threads(threads_num);
    }

    if (argc == 3) {
        max_num = atoi(argv[2]);
        assert(max_num <= 10000);
    }

    info(max_num);
    
    #pragma omp parallel
    {
        #pragma omp for reduction(+:sum) schedule(static)
        for(uint64_t i = 1; i <= max_num; i++) {
            printf("t: %d, %lu\n", omp_get_thread_num(), i);
            sum += i;
        }
    }

    printf("total sum: %lu\n", sum);
}

// we can't use shared(...) with #pragma omp for
// do not forgot {} for "for"???