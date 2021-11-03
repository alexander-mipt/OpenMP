#!/bin/bash

set -x

dirs=(hello for-schedule massive matrix teylor natural pqsort)

for i in ${dirs[@]}
do
	cd $i
	if [ -f a.out ]; then
		rm a.out
	fi
	cd ..
done

cd bugs/fixed/
for i in 1 2 3 4 5 6
do
	if [ -f "bugged$i.out" ]; then
		rm "bugged$i.out"
	fi
done 
cd ..