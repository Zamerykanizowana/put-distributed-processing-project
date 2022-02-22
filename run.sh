#!/bin/sh
BIN=main
NPROC="${1:-4}"

echo "Running with $NPROC processes!"

mpicc *.c -o $BIN -Wall -DLOG_USE_COLOR \
    && mpirun -np $NPROC --oversubscribe --mca opal_warn_on_missing_libcuda 0 ./$BIN
