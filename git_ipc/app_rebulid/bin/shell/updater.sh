#!/bin/sh

UPDATE_BASE="/tmp/upgrade/"
UPDATE_DIR="/tmp/upgrade/dev"

while [ true ]
do
	if [ -d ${UPDATE_DIR} ]; then
		
#
# Double Check
#
		sleep 1
		if [ -d ${UPDATE_DIR} ]; then
			echo -n "50" "${UPDATE_DIR}/rate"
			echo "Found!"	
			for file in `ls ${UPDATE_DIR}`
			do
				block="/dev/${file}"
				file="${UPDATE_DIR}/${file}"

				#if [ -f ${block} ]; then
					dd if\=${file} of\=${block}
				#else
				#	echo "${block} NOT Found!"
				#fi
			done
			echo -n "100" > "${UPDATE_BASE}/rate"
			break
		fi
	fi

#
# Check Every 1 Second
#
	sleep 5
done
