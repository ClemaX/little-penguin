#!/bin/sh

make

insmod main.ko
echo

echo 'cat -e /dev/fortytwo'
cat -e /dev/fortytwo
echo
echo

echo 'echo chamada > /dev/fortytwo'
echo chamada > /dev/fortytwo
echo

echo 'echo marvin > /dev/fortytwo'
echo marvin > /dev/fortytwo
echo

echo 'echo test >> /dev/fortytwo'
echo test >> /dev/fortytwo
echo

rmmod main.ko
echo

echo "Kernel log output:"

dmesg --since '30 seconds ago'

make clean
