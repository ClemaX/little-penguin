#!/bin/sh

make

insmod main.ko

echo 'cat -e /proc/mounpoints'
cat -e /proc/mountpoints

rmmod main.ko

make clean

dmesg --since "10 seconds ago"
