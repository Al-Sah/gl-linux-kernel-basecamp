#!/bin/bash

dir=""

if [ $# -ge 1 ] && [ -d "$1" ]; then
    dir="$1/"
fi

for (( i = 0; i < 10; i++ )); do
    touch "$dir""test$i.tmp"
done

for (( i = 0; i < 10; i++ )); do
    touch "$dir""_test$i.txt"
done

for (( i = 0; i < 10; i++ )); do
    touch "$dir""~test$i"
done