#include <algorithm>
#include <iostream>
#include <omp.h>

/* Regular quiksort algorithm, with the only exception that
 * the recursive step is done in parallel with openmp tasks
 */
void pqsort(int* vector, int first, int last) {
  if (first >= last)
    return; // no need to sort

  // otherwise select a pivot
  int i_pivot = (first + last) / 2;
  int pivot = vector[i_pivot];
  int left = first;
  int right = last;
  int temp = 0;
  while (left <= right) {
    if (vector[left] > pivot) { // swap left element with right element
      temp = vector[left];
      vector[left] = vector[right];
      vector[right] = temp;
      if (right == i_pivot) {
        i_pivot = left;
      }
      right--;
    } else {
      left++;
    }
  }
  // place the pivot in its place (i.e. swap with right element)
  temp = vector[right];
  vector[right] = pivot;
  vector[i_pivot] = temp;
// sort two sublists in parallel;

/* The recursive steps in quicksort execution is implemented as separate tasks
 */
#pragma omp task
  pqsort(vector, first, right - 1);
#pragma omp task
  pqsort(vector, right + 1, last);
}