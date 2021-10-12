#!/bin/bash

sudo dmesg -C

if [ -d "$(pwd)/build" ]; then
    cd "build" || exit
fi


sudo insmod ws2812_controller.ko pixels=32 brightness=10
sudo insmod mod_test.ko &> /dev/null
sudo rmmod ws2812_controller.ko

sudo insmod ws2812_controller.ko
sudo insmod mod_test.ko &> /dev/null
sudo rmmod ws2812_controller.ko

cd ..