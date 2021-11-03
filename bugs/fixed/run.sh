#!/bin/bash

set -x

for i in 1 2 3 4 5 6
do
	if [ -f "bugged$i" ]; then
		./bugged$i
	fi
done 