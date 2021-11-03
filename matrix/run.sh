#!/bin/bash

set -x

./matrix_gen.py --help -h
./a.out -h
time ./a.out 1 > output1.txt
time ./a.out 2 > output2.txt
diff output1.txt output2.txt