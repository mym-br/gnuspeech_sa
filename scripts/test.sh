#!/bin/sh

param_file=/tmp/gnuspeech_sa_param.txt
output_file=/tmp/gnuspeech_sa.wav

if [ $# -ne 1 ]
then
  echo "Usage: $0 'text'"
  exit 1
fi

./gnuspeech_sa -c ../gnuspeech_sa/data -p ${param_file} -o ${output_file} "$1" 2>/dev/null && aplay -q ${output_file}
