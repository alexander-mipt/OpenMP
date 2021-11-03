# OpenMP notes

[openmp](http://ccfit.nsu.ru/arom/data/openmp.pdf)

## Commands
```
gcc -fopenmp <src>
g++ -fopenmp <src>
```

## Notes

>`reduction` var should be `shared`. \
The var gets shared value outside the `for-loop` (for example, it will equal total sum).

> `omp sections` are not similar to `omp parallel`. Each thread execute some section only **once**. If some section has been already executed by another thread, the current thread passes it. \
> `nowait` disables explicit barrier synchronization (by default is exists in the end of `omp sections` module). 

> There is a simple rule to avoid deadlock: if exists ta, tb, La, Lb, and ta has set La and then sets Lb. ta must unset La before setting Lb which allows tb continue executing. 