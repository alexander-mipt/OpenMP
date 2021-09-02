#include<stdio.h>
#include<omp.h>
#include<stdint.h>

void init_omp() {
    // omp_set_dynamic(0);     // Explicitly disable dynamic teams
    omp_set_num_threads(4); // Use 4 threads for all consecutive parallel regions
}

void info() {
    printf("threads num max: %d\n", omp_get_max_threads());
}

int main() {
    // init_omp();
    info();

    uint64_t sum = 0;
    
#pragma omp parallel
{
    #pragma omp for reduction(+:sum)
    for(uint64_t i = 1; i <= 10; i++) {
        printf("t: %d, %lu\n", omp_get_thread_num(), i);
        sum += i;
    }
    printf("%lu\n", sum);
}
}

// we can't use shared(...) with #pragma omp for
// do not forgot {} for "for"???