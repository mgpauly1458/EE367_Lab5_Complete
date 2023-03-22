#!/bin/bash

files=("haha.txt" "large.txt")

for file in "${files[@]}"
do
  if cmp -s "t0/$file" "t1/$file"; then
    echo "t0/$file equal to t1/$file"
  else
    echo "Error: $file is not equal"
    exit 1
  fi
done

echo "All files are equal"

