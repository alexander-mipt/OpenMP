# Massive Change
```
./a.out --help
```
## build
```
g++ -fopenmp massive_change
```

## run
```
time ./a.out 10000000 1 > result1.txt
time ./a.out 10000000 100 > result100.txt
diff result100.txt result1.txt 
```