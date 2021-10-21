# Taylor calculator
## build
```
g++ -fopenmp teylor.cpp
```

## run
```
./a.out -h
./a.out 30
./a.out 30 10
./a.out 30 100
```

Текущий ряд - `sin(1)` ~ $0.841470...$ \
Также можно вычислить `exp(1)` ~ $2.71828...$ \
или добавить любую другую функцию, удовлетворяющую шаблону функции `teylor_mem`