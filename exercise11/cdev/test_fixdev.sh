#!/bin/bash

echo " * test: Before insmod ... "
sudo dmesg -C

sudo insmod fixdev.ko major=333
dmesg

echo " * test: Creating files /dev/zzzz*"
sudo mknod -m0666 /dev/zzzz c 333 0
sudo mknod -m0666 /dev/zzzz1 c 333 1
sudo mknod -m0666 /dev/zzzz2 c 333 2
sudo mknod -m0666 /dev/zzzz3 c 333 3

echo " * test: Reading files /dev/zzzz*"
sudo cat /dev/zzzz
sudo cat /dev/zzzz1
sudo cat /dev/zzzz2
sudo cat /dev/zzzz3

echo " * test: Deleting files /dev/zzzz*"
sudo rm /dev/zzzz*
ls -al /dev/zz*

echo " * test: Working with file zzzz1"
mknod -m0666 zzzz1 c 333 0
cat zzzz1
rm zzzz1

echo " * test: Removing module"
sudo rmmod fixdev

echo " * test: Check random major value"
sudo insmod fixdev.ko
sudo rmmod fixdev
dmesg

sudo dmesg -C



