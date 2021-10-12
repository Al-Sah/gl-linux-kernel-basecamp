#!/bin/bash

sudo dmesg -C

if [ -d "$(pwd)/build" ]; then
    cd "build" || exit
fi

sudo insmod ws2812_controller.ko

sudo insmod mod_test.ko &> /dev/null

sudo rmmod ws2812_controller.ko

cd ..