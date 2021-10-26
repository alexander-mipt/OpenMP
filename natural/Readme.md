# build
```
g++ -fopenmp natural.cpp
```

# run
```
time ./a.out 100000000 5 > result5.txt # 45s
time ./a.out 100000000 1 > result1.txt # 33s
diff result1.txt result5.txt 
```

Применен алгоритм 