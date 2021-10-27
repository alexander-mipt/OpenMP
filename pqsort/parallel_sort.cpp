#include <omp.h>
#include <algorithm>
#include <iostream>

/*!
 * quick_sort - параллельная реализация алгоритма быстрой сортировки
 * @param a сортируемый массив
 * @param n индекс последнего элемента массива (не размер массива!)
 */
void quick_sort(int x0, int *a, const long n) {
  long i = 0, j = n;
	float	pivot = a[n / 2]; // опорный элемент
	
#ifdef DEBUG
	#pragma omp critical
		{
			printf("enter: %d\n", x0);
		}
#endif

	do {
		while (a[i] < pivot) i++;
		while (a[j] > pivot) j--;

		if (i <= j) {
      std::swap(a[i], a[j]);
			i++; j--;
		}
	} while (i <= j);

#ifdef DEBUG
	#pragma omp critical
		{
			printf("divide #%d by %3.2f [%d] -> (%d|%d) : %d\n", x0, pivot, n/2, j, n - i, n);
		}
#endif

	if (n < 100) {
		if (j > 0) quick_sort(x0 * 2 + 1, a, j);
		if (n > i) quick_sort(x0 * 2 + 2, a + i, n - i);
		return;
	}
	
#pragma omp task shared(a)
	{
        printf("t: %d\n", omp_get_thread_num());
		if (j > 0) quick_sort(x0 * 2 + 1, a, j);
	} // #pragma omp task
#pragma omp task shared(a)
	{
		if (n > i) quick_sort(x0 * 2 + 2, a + i, n - i);
	} // #pragma omp task
#pragma omp taskwait
}