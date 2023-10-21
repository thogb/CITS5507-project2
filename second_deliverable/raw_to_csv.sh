#!/bin/sh

# Retrieved from project 1

IN_FILE=$1
OUT_FILE=$2

if [[ -z "$IN_FILE" || -z "$OUT_FILE" ]]
then
    echo "Usage: $0 <input_file> <output_file>"
    exit 1
fi

awk -F', ' -f raw_to_csv.awk $IN_FILE > $OUT_FILE