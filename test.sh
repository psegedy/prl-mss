#!/bin/bash
# $1 - number of numbers
# $2 - number of processors

numbers=$1
cpus=$2

# check if number of numbers is divisible by number of CPUs
if ((numbers % cpus)); then
    # use smaller number of CPUs
    while ((numbers % cpus)); do
        ((cpus--))
    done
    # echo "CPUs = $cpus"
fi

# compile
mpic++ -g --prefix /usr/local/share/OpenMPI -o mss mss.cpp

# make file with random numbers
dd if=/dev/random bs=1 count=$numbers of=numbers &> /dev/null

# run
mpirun --prefix /usr/local/share/OpenMPI -np $cpus mss $numbers

rm -f mss numbers
