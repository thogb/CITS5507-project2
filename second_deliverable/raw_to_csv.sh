#!/bin/sh

IN_FILE=$1

if [[ -z "$IN_FILE" ]]
then
    echo "Usage: $0 <input_file>"
    exit 1
fi

OUT_FILE="$1.csv"

awk -F', ' -f raw_to_csv.awk $IN_FILE > $OUT_FILE