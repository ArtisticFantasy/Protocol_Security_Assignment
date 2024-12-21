#!/bin/bash
source $(dirname $0)/env_setup.sh

while getopts i:o:p:l: flag
do
    case "${flag}" in
        i) input_file=${OPTARG};;
        o) output_file=${OPTARG};;
        p) pred_pattern_file=${OPTARG};;
        l) prefix_len=${OPTARG};;
    esac
done

if [ -z "$input_file" ] || [ -z "$output_file" ] || [ -z "$pred_pattern_file" ] || [ -z "$prefix_len" ]; then
    echo "Usage: $0 -i input_file -o output_file -p pred_pattern_file -l prefix_len"
    exit 1
fi

${PROJECT_DIR}/gps -i ${input_file} \
-o ${output_file} \
-p ${pred_pattern_file} \
-l ${prefix_len} \
--part2