#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdint.h>
#include <stdbool.h>
#include "assert.h"
#include <unistd.h>

#define DEBUG_ENABLED 1

#ifdef DEBUG_ENABLED
#define DEBUG(x) x
#else
#define DEBUG(x) {}
#endif

#define SIZE 100000
#define EPSILON 1e-6

typedef uint64_t elem;

void init_omp(int threads_num) {
    // omp_set_dynamic(0);     // Explicitly disable dynamic teams
    omp_set_num_threads(threads_num); // Use 4 threads for all consecutive parallel regions
}

void init_array(elem* const array, const unsigned size) {
    for (unsigned i = 0; i < size; ++i) {
        array[i] = i + 1.0;
    }
}

void print_array(elem* const array, const unsigned size) {
    for (unsigned i = 0; i < size; ++i) {
        printf("%lu ", array[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    assert(argc <= 2);
    unsigned threads = 4;
    if (argc == 2) {
        threads = atoi(argv[1]);
    }
    
    elem a[SIZE] = {};
    elem b[SIZE] = {};
    init_array(a, SIZE);
    print_array(a, SIZE);

    init_omp(threads);
    
    #pragma omp parallel shared(a) shared(b)
    {
        #pragma omp for
        for(unsigned i = 1; i < SIZE - 1; ++i) {
            b[i] = a[i - 1] * a[i] * a[i + 1] / 3.0;
        }
    }
    b[0] = a[0];
    b[SIZE - 1] = a[SIZE - 1];

    print_array(b, SIZE);
}

// we can't use shared(...) with #pragma omp for

// 101 (25, 4t): 25(0), 25(1), 25(2), 25(3), 1(0)
// 10^-308 