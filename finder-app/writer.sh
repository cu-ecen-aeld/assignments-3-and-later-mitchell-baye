#!/bin/sh

if [ $# -ne 2 ]; then
    echo "Error: Wrong number of arguments"
    exit 1
fi

path=$1
full_path="${path%/*}"

mkdir -p $full_path
touch_file=$(touch $1 && echo "$2" > $1)

if [[ $? -eq 1 ]]; then
    echo "Error: File could not be created"
    exit 1
fi
