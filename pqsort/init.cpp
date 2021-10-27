#include "init.hpp"
#include <cstdio>
#include <cstring>
#include <omp.h>
#include <cstdlib>

void thread_info() {
  printf("info:\n");
  printf("\tthreads num set: %d\n", omp_get_max_threads());
  printf("\tnum of cores: %d\n", omp_get_num_procs());
  printf("\tdynamic threads allowed? %d\n", omp_get_dynamic());
  printf("\tnested allowed? %d\n", omp_get_nested());
  printf("\tthread limit constant: %d\n\n", omp_get_thread_limit());
}

void init_omp(int threads_num = 1) {
  omp_set_dynamic(1); // Explicitly enable dynamic teams
  omp_set_nested(1);
  omp_set_num_threads(threads_num);
  thread_info();
}

void parse_help(int argc, char *argv[], const char *message) {
  for (int i = 0; i < argc; ++i) {
    const char *help = argv[i];
    if (!strcmp("-h", help) || !strcmp("--help", help)) {
      printf("%s\n", message);
      exit(0);
    }
  }
}