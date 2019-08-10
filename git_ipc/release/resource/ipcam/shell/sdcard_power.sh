#!/bin/sh

#enable gpio4_0
/media/custom/bin/himm 0x1204006c 0x0

/media/custom/bin/himd.l 0x1204006c

#export gpio4_0  id=4*8+0=32
echo 32 > /sys/class/gpio/export

#set gpio4_0 direction
echo in > /sys/class/gpio/gpio32/direction

#get gpio4_0 value
regValue=$(cat /sys/class/gpio/gpio32/value)

if [ $regValue == "1" ]; then
	echo "SDIO_POWER_EN input value 1"
	echo out > /sys/class/gpio/gpio32/direction
	echo 1 > /sys/class/gpio/gpio32/value
	sleep 0.2
	echo 0 > /sys/class/gpio/gpio32/value
	echo "set SDIO_POWER_EN output value 0"
else
	echo "SDIO_POWER_EN input value 0"
	echo out > /sys/class/gpio/gpio32/direction
	echo 0 > /sys/class/gpio/gpio32/value
	sleep 0.2
	echo 1 > /sys/class/gpio/gpio32/value
	echo "set SDIO_POWER_EN output value 1"
fi

#unexport gpio4_0
echo 32 > /sys/class/gpio/unexport

echo "load himciv200.ko"
insmod /media/custom/ipcam/mpp/extdrv/himciv200.ko
