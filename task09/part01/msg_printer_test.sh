#!/bin/bash

sudo insmod messages_printer.ko

echo "zzz1" | sudo tee /proc/messages_printer/data
echo "zzz2" | sudo tee /proc/messages_printer/data
echo "zzz3" | sudo tee /proc/messages_printer/data
echo "zzz4" | sudo tee /proc/messages_printer/data

