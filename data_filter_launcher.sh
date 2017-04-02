#!bin/bash

env DATA_SIZE = "70K"
env INPUT_DATA = "/dev/urandom"
env OUTPUT_DATA = "/home/ariel/Documents/Tel\ Aviv\ University/Comuputer\ Science/Operation\ Systems/HW1/output.txt"
gcc data_filter.c -o data_filter
./data_filter DATA_SIZE INPUT_DATA OUTPUT_DATA