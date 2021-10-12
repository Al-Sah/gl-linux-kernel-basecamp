#!/bin/bash

sudo insmod messages_printer.ko

echo "zzz1" | sudo tee /proc/messages_printer/data
echo "zzz2" | sudo tee /proc/messages_printer/data
echo "zzz3" | sudo tee /proc/messages_printer/data
echo "zzz4" | sudo tee /proc/messages_printer/data

echo "300 1000" | sudo tee /sys/class/messages_printer/settings

echo "zzz1" | sudo tee /proc/messages_printer/data
echo "zzz2" | sudo tee /proc/messages_printer/data
echo "zzz3" | sudo tee /proc/messages_printer/data
echo "zzz4" | sudo tee /proc/messages_printer/data

#sudo rmmod messages_printer.ko