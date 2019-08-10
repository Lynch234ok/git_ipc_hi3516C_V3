#!/bin/sh


while [ 1 ]
do
	if [ "$2" -eq 1 ]; then
		/usr/share/ipcam/shell/IOTDaemon  -S "JA$1" -L /media/tf/
	else
		/usr/share/ipcam/shell/IOTDaemon  -S "JA$1"
	fi
	sleep 1
done

