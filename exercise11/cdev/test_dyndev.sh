#!/bin/bash


dmesg -C

echo " * test: Before insmod ... "

echo " * test run command: ls -al /dev/dyn_* "
ls -al /dev/dyn_*

echo " * test run command: insmod dyndev.ko "
insmod dyndev.ko

echo "After insmod ... "
echo " * test run command: ls -al /dev/dyn_* "
ls -al /dev/dyn_*

echo " * test run command: cat devices | grep dyn "
cat devices | grep dyn

echo " * test run command: cat /proc/modules | grep dyn "
cat /proc/modules | grep dyn

echo " * test: Reading files /dev/dyn_*"
cat /dev/dyn_0
cat /dev/dyn_1
cat /dev/dyn_2

echo " * test run command: dmesg "
dmesg

echo " * test run command: ls -al /sys/class/dyn_class "
ls -al /sys/class/dyn_class

echo " * test run command: ls -al /sys/class/dyn_class "
tree /sys/devices/virtual/dyn_class/dyn_1

echo " * test run command: rmmod dyndev"
rmmod dyndev

echo "After rmmod ... "
echo " * test run command: ls -al /dev/dyn_*"
ls -al /dev/dyn_*





