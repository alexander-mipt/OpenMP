#include <cstdio>
#include <cstdlib>
#include <omp.h>
#include <stdint.h>
#include <cassert>
#include <cstring>

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
    const char* message = \
    "Sum <N> elems with <P> threads.\n"
    "\tArgs: <N> [<P>] (default 1 thread)";

    parse_help(argc, argv, message);

    assert(argc > 1);

    long N = atol(argv[1]);
    assert(N > 0);

    int threads = 1;
    if (argc == 3) {
        threads = atoi(argv[2]);
        assert(threads > 0 && threads < omp_get_thread_limit());
    }

    init_omp(threads);

    uint64_t sum = 0;
    
    #pragma omp parallel shared(sum)
    {
        #pragma omp for reduction(+:sum) schedule(static)
        for(uint64_t i = 1; i <= N; i++) {
            // printf("t: %d, %lu\n", omp_get_thread_num(), i);
            sum += i;
        }
    }

    printf("total sum: %lu\n", sum);
}

// we can't use shared(...) with #pragma omp for
// do not forgot {} for "for"???