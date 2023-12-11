#!/bin/sh

make

insmod ex07.ko
mount -vt debugfs none /sys/kernel/debug
echo

echo 'ls -l /sys/kernel/debug/fortytwo'
ls -l /sys/kernel/debug/fortytwo
echo
echo

echo 'echo chamada > /sys/kernel/debug/fortytwo/id'
echo chamada > /sys/kernel/debug/fortytwo/id
echo

echo 'echo marvin > /sys/kernel/debug/fortytwo/id'
echo marvin > /sys/kernel/debug/fortytwo/id
echo

echo 'echo test >> /sys/kernel/debug/fortytwo/id'
echo test >> /sys/kernel/debug/fortytwo/id
echo

echo 'cat /sys/kernel/debug/fortytwo/jiffies'
cat /sys/kernel/debug/fortytwo/jiffies
echo

echo 'echo test > /sys/kernel/debug/fortytwo/foo'
echo test > /sys/kernel/debug/fortytwo/foo
echo

echo 'cat /sys/kernel/debug/fortytwo/foo'
cat /sys/kernel/debug/fortytwo/foo
echo

echo 'echo test > /sys/kernel/debug/fortytwo/foo'
echo test >> /sys/kernel/debug/fortytwo/foo
echo

echo 'cat /sys/kernel/debug/fortytwo/foo'
cat /sys/kernel/debug/fortytwo/foo
echo

rmmod ex07.ko
umount -v /sys/kernel/debug
echo

echo "Kernel log output:"

dmesg --since '30 seconds ago'

make clean
