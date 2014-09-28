#!/bin/sh

param_file=/tmp/gnuspeech_x_param.txt
output_file=/tmp/gnuspeech_x.wav

if [ $# -ne 1 ]
then
  echo "Usage: $0 'text'"
  exit 1
fi

LD_LIBRARY_PATH=. ./gnuspeech_x -v -c ../data -p ${param_file} -o ${output_file} "$1" && aplay ${output_file}
