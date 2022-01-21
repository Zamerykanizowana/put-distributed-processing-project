#!/bin/sh
BIN=main
mpicc *.c -o $BIN -Wall -DLOG_USE_COLOR \
    && mpirun --mca opal_warn_on_missing_libcuda 0 ./$BIN
