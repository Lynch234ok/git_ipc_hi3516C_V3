#!/bin/sh

ulimit -s 64

while [ 1 ]
do
	if [ "$2" -eq 1 ]; then
		/media/custom/ipcam/shell/IOTDaemon  -S "JA$1" -L /media/tf/
	else
		/media/custom/ipcam/shell/IOTDaemon  -S "JA$1"
	fi
	sleep 1
done

