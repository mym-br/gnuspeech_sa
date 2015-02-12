#!/bin/sh

param_file=/tmp/gnuspeech_sa_param.txt
output_file=/tmp/gnuspeech_sa.wav

if [ $# -ne 1 ]
then
  echo "Usage: $0 'text'"
  exit 1
fi

./gnuspeech_sa -c ../gnuspeech_sa/data/en -p ${param_file} -o ${output_file} "$1" && aplay -q ${output_file}
