#!/bin/sh

cleanup()
{
	echo

	rm -v /etc/udev/rules.d/99-ft.rules

	udevadm control --reload

	make clean
}

trap cleanup INT

make

cp -v 99-ft.rules /etc/udev/rules.d
udevadm control --reload

dmesg --since '1 second ago' -w
