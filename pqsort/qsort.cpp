#include "omp.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pqsort.hpp"

int lenArr = 28; // 262144000; // 262144000 = 1000 MBytes = 1 GByte
int numthreads = 2;

// Comparator used in qsort()
int cmpfunc(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

int main() {
  int minMum = 1;
  int maxNum = lenArr;
  int maxNumbersDisplayed = 30;

  double startTime = 0.;
  double stopTime = 0.;

  int *arr1;
  int *arr2;
  int *arr3;
  arr1 = (int *)malloc(lenArr * sizeof(int));
  arr2 = (int *)malloc(lenArr * sizeof(int));

  // Initialise the array with random numbers
  srand(5); // seed
  printf("Initializing the arrays with random numbers...\n");
  for (int i = 0; i < lenArr; i++) {
    arr1[i] = minMum + (rand() % maxNum);
    arr2[i] = arr1[i];
  }

  if (lenArr <= maxNumbersDisplayed) {
    printf("\n\nArray BEFORE sorting:\n");
    for (int i = 0; i < lenArr; i++) {
      printf("%d ", arr1[i]);
    }
    printf("\n\n\n");
  }

  printf("Sorting with serial 'qsort' function of 'stdlib.h' ...\n");
  startTime = clock();
  qsort(arr1, lenArr, sizeof(int), cmpfunc);
  stopTime = clock();

  if (lenArr <= maxNumbersDisplayed) {
    for (int i = 0; i < lenArr; i++) {
      printf("%d ", arr1[i]);
    }
  }
  printf("\nSorted in (aprox.): %f seconds \n\n",
         (double)(stopTime - startTime) / CLOCKS_PER_SEC);

  printf("Sorting with custom PARALLEL QuickSort... \n");
  startTime = omp_get_wtime();
  quickSort_parallel(arr2, lenArr, numthreads);
  stopTime = omp_get_wtime();

  if (lenArr <= maxNumbersDisplayed) {
    for (int i = 0; i < lenArr; i++) {
      printf("%d ", arr2[i]);
    }
  }
  printf("\nSorted in (aprox.): %f seconds \n\n",
         (double)(stopTime - startTime) / CLOCKS_PER_SEC);

  printf("\nArray comparison\n");
  bool correctResult;

  correctResult = true;
  int i = 0;
  while (correctResult == true && i < lenArr) {
    if (arr1[i] != arr2[i]) {
      correctResult = false;
    }
    ++i;
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