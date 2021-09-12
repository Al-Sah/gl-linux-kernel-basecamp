#!/bin/bash

dir=""

if [ $# -ge 1 ] && [ -d "$1" ]; then
    dir="$1/"
fi

for (( i = 0; i < 10; i++ )); do
    touch -t 201212101830.55 "$dir""test$i.tmp"
done

