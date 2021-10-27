#include "omp.h"

#include <cstdio>
#include <cstdlib>

#include "init.hpp"
#include "parallel_sort.hpp"
#include <cassert>

constexpr int print_lim = 32;

// Comparator used in qsort()
int cmpfunc(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

int main(int argc, char **argv) {

  const char *message = "Sorts 2 arrays with <N> nums in [<min>, <max>] with 1"
                        "and <P> threads respec.\n"
                        "\tArgs: <N> <min> <max> [<P>] (default 1 thread)";

  parse_help(argc, argv, message);

  assert(argc > 3);
  const long N = atol(argv[1]);
  assert(N > 0);
  int min = atoi(argv[2]);
  int max = atoi(argv[3]);
  assert(max >= min);

  int threads = 1;
  if (argc == 5) {
    threads = atoi(argv[4]);
    assert(threads > 0 && threads < omp_get_thread_limit());
  }

  int *arr1 = (int *)malloc(N * sizeof(int));
  int *arr2 = (int *)malloc(N * sizeof(int));

  // Initialise the array with random numbers
  srand(5); // seed
  printf("Initializing the arrays with random numbers...\n");
  for (int i = 0; i < N; ++i) {
    arr1[i] = min + (rand() % max);
    arr2[i] = arr1[i];
  }

  printf("\nBefore sorting:\n");
  for (int i = 0; N <= print_lim && i < N; i++) {
    printf("%d ", arr1[i]);
  }
  printf("\n");

  printf("Sorting with serial 'qsort' function of 'stdlib.h' ...\n");
  double dt = 0.;
  dt = omp_get_wtime();
  qsort(arr1, N, sizeof(int), cmpfunc);
  dt = omp_get_wtime() - dt;

  for (int i = 0; N < print_lim && i < N; ++i) {
    printf("%d ", arr1[i]);
  }
  printf("\ntime: %lf sec\n", dt);

  printf("Sorting with PARALLEL quick_sort ...\n");
  init_omp(threads);
  dt = omp_get_wtime();
  #pragma omp parallel shared(arr2)
	{
		#pragma omp single nowait 
		{
			quick_sort(0, arr2, N);
		} // #pragma omp single
	} // #pragma omp parallel
  dt = omp_get_wtime() - dt;

  for (int i = 0; N < print_lim && i < N; ++i) {
    printf("%d ", arr2[i]);
  }
  printf("\ntime: %f sec\n", dt);

  printf("Checking ...\n");
  bool correctResult;

  correctResult = true;
  for (int i = 0; i < N && correctResult == true; ++i) {
    if (arr1[i] != arr2[i]) {
      correctResult = false;
    }
  }

  if (correctResult == true) {
    printf("Arrays are equal\n");
  } else {
    printf("Arrays are not equal\n");
  }

  printf("\n\n");

  free(arr1);
  free(arr2);
  return 0;
}