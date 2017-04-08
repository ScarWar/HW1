#!bin/bash

export DATA_SIZE=30K
export INPUT_DATA=/dev/urandom	
export OUTPUT_DATA=/a/home/cc/students/cs/bereslavsky/HW1/output.txt	
./data_filter $DATA_SIZE $INPUT_DATA $OUTPUT_DATA
