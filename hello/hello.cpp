#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdint.h>
#include "assert.h"
#include "string.h"

void init_omp(int threads_num = 1) {
    // omp_set_dynamic(threads_num);     // Explicitly disable dynamic teams
    omp_set_num_threads(threads_num);
}

void thread_info() {
    printf("info:\n");
    printf("\tthreads num set: %d\n", omp_get_max_threads());
    printf("\tnum of cores: %d\n", omp_get_num_procs());
    printf("\tdynamic threads allowed? %d\n", omp_get_dynamic());
    printf("\tnested allowed? %d\n", omp_get_nested());
    printf("\tthread limit constant: %d\n", omp_get_thread_limit());
}

void parse_help(int argc, char* argv[], const char* message) {
    for (int i = 0; i < argc; ++i) {
        const char* help = argv[i];
        if (!strcmp("-h", help) || !strcmp("--help", help)) {
            printf("%s\n", message);
            exit(0);
        }
    }
}

int main(int argc, char* argv[]) {
    uint32_t threads_num = 1;
    const char* message = \
    "Prints \"hello\" from ordered threads.\n"
    "\tArgs: [<num threads>]";

    parse_help(argc, argv, message);
    assert(argc <= 2);

    if (argc == 2) {
        threads_num = atoi(argv[1]);
        assert(threads_num > 0);
    }
    
    init_omp(threads_num);

    thread_info();

    uint32_t queue = omp_get_max_threads() - 1;
    
    // direct order
    /*
    #pragma omp parallel
    {
        #pragma omp for ordered
        for (int i = 0; i < omp_get_max_threads(); ++i)
            #pragma omp ordered
            printf("Hello from tid %d\n", omp_get_thread_num());    
    }
    */

    #pragma omp parallel shared(queue)
    {   
        int myid = omp_get_thread_num();
        while (queue != myid);
        printf("Hello from tid %d\n", myid);
        queue --;
    }
}
// ordered - must be written in omp for and inside for