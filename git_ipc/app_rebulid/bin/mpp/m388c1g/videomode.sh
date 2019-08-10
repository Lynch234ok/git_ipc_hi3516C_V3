#!/bin/sh
cd /usr/share/ipcam/mpp/
SENDER=/usr/share/ipcam/mpp/mstrm_msg_sender
export LD_LIBRARY_PATH=/usr/share/ipcam/mpp/lib:$LD_LIBRARY_PATH
while [ 1 ]
	do
	#echo "/usr/share/ipcam/mpp/mstrm_msg_sender -C 0 -c 0 -a $(($RANDOM%3)) -m $(($RANDOM%11))"
	$SENDER -C 0 -c 0 -a $(($RANDOM%3)) -m $(($RANDOM%11))
	sleep 10
	done
