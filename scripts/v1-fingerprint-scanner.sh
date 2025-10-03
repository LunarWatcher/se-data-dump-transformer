#!/usr/bin/bash

DATA_DUMP_DIR=$1

if [[ "$DATA_DUMP_DIR" == "" ]]; then
    echo "Usage: v1-fingerprint-scanner path-to-data-dump-root"
    exit -1
fi

if [[ "$2" != "" ]]; then
    echo "Found unexpected trailing arguments: $2"
    echo "Are you missing quotes around your path?"
    exit -2
fi

echo "Scanning $DATA_DUMP_DIR"

for filename in "${DATA_DUMP_DIR}"/*.7z; do
    echo "Checking $filename"
    7z x -so "$filename" Posts.xml | grep -E 'Id="10000000[01][01]"' && echo "$filename is bad"
done

