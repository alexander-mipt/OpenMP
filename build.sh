#!/bin/bash

set -x

build="g++ -fopenmp"

dirs=(hello for-schedule massive matrix teylor natural)

for i in ${dirs[@]}
do
	cd $i
	if [ ! -f a.out ]; then
		src=`find . -name *.c*`
		echo "building $src"
		$build $src
	fi
	cd ..
done

cd pqsort
if [ ! -f a.out ]; then
	g++ -fopenmp init.cpp parallel_sort.cpp qsort.cpp
fi
cd ..

cd bugs/fixed/
for i in 1 2 3 4 5 6
do
	if [ ! -f "bugged$i.out" ]; then
		g++ -fopenmp "bugged$i.c" -o "bugged$i.out"
	fi
done 
cd ..