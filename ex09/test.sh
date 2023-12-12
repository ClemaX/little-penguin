#!/bin/sh

make

insmod main.ko
sleep 1

rmmod main.ko
sleep 0.5

make clean

dmesg --since "10 seconds ago"
