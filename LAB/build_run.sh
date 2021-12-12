#!/bin/bash
#PATH=~/Tar/openmpi-4.1.0/build/bin/:$PATH

#set -x

mpicc parallel.c -O3 -std=c99 -lm -o parallel
#time mpirun --hostfile hostfile -np $1 ./a.out
#mpicc sequential.c -o sequential -lm

#rm -f antipov_job.*
qsub job1.sh
#sleep 2
#cat antipov_job.*
