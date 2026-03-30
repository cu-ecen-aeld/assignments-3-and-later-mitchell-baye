#!/bin/sh

if [ "$#" -ne 2 ]; then
    echo "Error: Wrong number of arguments provided."
    exit 1 
fi

if [ ! -d "$1" ]; then
    echo "Error: First arugment is not a directory"
    exit 1
fi

num_files=$(find $1 -type f | wc -l)
num_lines=$(grep -r $2 $1 | wc -l)
echo "The number of files are $num_files and the number of matching lines are $num_lines"
