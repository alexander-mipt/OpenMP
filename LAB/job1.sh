#!/bin/bash

#PBS -l walltime=00:00:30,nodes=2:ppn=4
#PBS -N antipov_job
#PBS -q batch

cd $PBS_O_WORKDIR
#mpirun --hostfile $PBS_NODEFILE -np $PROCS ./parallel
for (( i = 1; i <=32 ; ++i )); do
	time mpirun --hostfile $PBS_NODEFILE -np $i ./parallel
done
