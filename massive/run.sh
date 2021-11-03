#!/bin/bash

set -x

./a.out -h
time ./a.out 10000000 1 > result1.txt
time ./a.out 10000000 100 > result100.txt
diff result100.txt result1.txt 