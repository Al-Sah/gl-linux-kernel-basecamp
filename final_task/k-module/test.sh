#!/bin/bash

sudo dmesg -C

if [ -d "$(pwd)/build" ]; then
    cd "build" || exit
fi

sudo insmod rpi_ws2812matrix_master.ko

sudo insmod mod_test.ko &> /dev/null
sleep 2
sudo insmod mod_test.ko &> /dev/null

sudo rmmod rpi_ws2812matrix_master.ko

cd ..