#!/bin/bash

sudo dmesg -C

if [ -d "$(pwd)/build" ]; then
    cd "build" || exit
fi

sudo insmod matrix_controller.ko

sudo insmod mod_test.ko &> /dev/null
sleep 2
sudo insmod mod_test.ko &> /dev/null

sudo rmmod matrix_controller.ko

cd ..