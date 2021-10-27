#!/bin/bash

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
