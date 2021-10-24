#include <omp.h>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <sys/types.h>
#include <vector>
#include <iostream>

using num = unsigned long long;
constexpr bool undefined = true;
constexpr bool excluded = false;

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

int main(int argc, char** argv) {
    const char* message = \
    "Calculates simple nums up to <N> with <P> threads.\n"
    "\tArgs: <N> [<P>] (default 1 thread)";

    parse_help(argc, argv, message);

    assert(argc > 1);

    num N = atoi(argv[1]);
    std::cout << N  << std::endl;
    // assert(N > 0);

    int threads = 1;
    if (argc == 3) {
        threads = atoi(argv[2]);
        assert(threads > 0 && threads < omp_get_thread_limit());
    }

    std::vector<bool> prime(N, undefined);
    init_omp(threads);

    #pragma omp parallel //shared(prime)
    {
        num start_num = omp_get_thread_num() + 2;
        getchar();
        bool end = false;
        while(!end) {
            //std::cerr << start_num << std::endl;
            for (auto i = 2*start_num; i <= prime.size(); i += start_num) {
                prime[i - 1] = excluded;
            }
            (start_num >= prime.size())? end = true : ++start_num;
            while (start_num < prime.size() && prime[start_num - 1] == excluded) {
                ++start_num;
            }
        }
    }

    for (num i = 1; i <= prime.size(); ++i) {
        if (prime[i - 1] != excluded) {
            std::cout << i << "\n";
        }
    }

}