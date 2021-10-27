#include <cstdio>
#include <cstdlib>
#include <omp.h>
#include <cstdint>
#include <cassert>
#include <unistd.h>
#include <cstring>
#include <vector>

#define DEBUG_ENABLED 1

#ifdef DEBUG_ENABLED
#define DEBUG(x) x
#else
#define DEBUG(x) {}
#endif

#define EPSILON 1e-6

typedef uint64_t elem;

void thread_info() {
    printf("info:\n");
    printf("\tthreads num set: %d\n", omp_get_max_threads());
    printf("\tnum of cores: %d\n", omp_get_num_procs());
    printf("\tdynamic threads allowed? %d\n", omp_get_dynamic());
    printf("\tnested allowed? %d\n", omp_get_nested());
    printf("\tthread limit constant: %d\n\n", omp_get_thread_limit());
}

void init_omp(int threads_num = 1) {
    // omp_set_dynamic(threads_num);     // Explicitly disable dynamic teams
    omp_set_num_threads(threads_num);
    thread_info();
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

void init_array(std::vector<elem>& array) {
    for (auto i = 0; i < array.size(); ++i) {
        array[i] = i + 1.0;
    }
}

void print_array(std::vector<elem>& array) {
    for (auto i = 0; i < array.size(); ++i) {
        printf("%lu ", array[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {

    const char* message = \
    "Calculates new array size of <N> with <P> threads.\n"
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
    
    std::vector<elem> a(N, 0.);
    std::vector<elem> b(N, 0.);

    init_array(a);
    print_array(a);
    
    #pragma omp parallel shared(a) shared(b)
    {
        #pragma omp for
        for(auto i = 1; i < N - 1; ++i) {
            b[i] = a[i - 1] * a[i] * a[i + 1] / 3.0;
        }
    }
    b[0] = a[0];
    b[N - 1] = a[N - 1];

    print_array(b);
}

// we can't use shared(...) with #pragma omp for

// 101 (25, 4t): 25(0), 25(1), 25(2), 25(3), 1(0)
// 10^-308 