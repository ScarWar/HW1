#!bin/bash

export DATA_SIZE=12M
export INPUT_DATA=/dev/urandom
export OUTPUT_DATA=/a/home/cc/students/cs/bereslavsky/HW1/output.txt
gcc data_filter.c -o data_filter
data_filter $DATA_SIZE $INPUT_DATA $OUTPUT_DATA
