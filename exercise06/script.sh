#!/bin/bash

if [ -f "/sys/class/gpio/gpio26" ]; then
    echo " GPIO26 is active ! "
    echo 26 > /sys/class/gpio/unexport
fi

echo 26 > /sys/class/gpio/export
echo high > /sys/class/gpio/gpio26/direction
echo 1 > /sys/class/gpio/gpio26/active_low
echo "GPIO26 is ready !!"