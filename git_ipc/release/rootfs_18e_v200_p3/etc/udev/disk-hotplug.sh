#!/bin/sh

################### SCSI disk #################################################
#DEVTYPE=partition 
#DEVPATH=/devices/platform/ahci.0/ata1/host0/target0:0:0/0:0:0:0/block/sdb/sdb2 
#SUBSYSTEM=block 
#SEQNUM=800 
#UDEVD_EVENT=1 
#ACTION=remove 
#DEVNAME=/dev/sdb1 
#DEVTYPE=partition 
#DEVPATH=/devices/platform/ahci.0/ata1/host0/target0:0:0/0:0:0:0/block/sdb/sdb1 
#SUBSYSTEM=block 
#SEQNUM=801 
#UDEVD_EVENT=1 
#ACTION=remove 
#DEVNAME=/dev/sdb 
#DEVTYPE=disk 
#DEVPATH=/devices/platform/ahci.0/ata1/host0/target0:0:0/0:0:0:0/block/sdb 
#SUBSYSTEM=block 
#SEQNUM=803 
#UDEVD_EVENT=1 

################### USB disk #################################################
#
# Block device event:
#
# ACTION=add 
# DEVNAME=/dev/sda 
# DEVTYPE=disk 
# DEVPATH=/devices/platform/hiusb-ehci.0/usb1/1-2/1-2.1/1-2.1:1.0/host8/target8:0:0/8:0:0:0/block/sda 
# SUBSYSTEM=block 
# SEQNUM=544 
# UDEVD_EVENT=1 
# 
# ACTION=add 
# DEVNAME=/dev/sda1 
# DEVTYPE=partition 
# DEVPATH=/devices/platform/hiusb-ehci.0/usb1/1-2/1-2.1/1-2.1:1.0/host8/target8:0:0/8:0:0:0/block/sda/sda1 
# SUBSYSTEM=block 
# SEQNUM=545 
# UDEVD_EVENT=1 
#
# Use command "hdparm -z /dev/sda" to reread partition table
#

################################################################################
PREFIX=udisk
CONSOLE=/dev/ttyS000
################################################################################
show_env ()
{
	local RED="\033[31m"
	local NORMAL="\033[00m"
	{
		echo -e ${RED}"ACTION=$ACTION"           ${NORMAL}
		echo -e ${RED}"DEVNAME=$DEVNAME"         ${NORMAL}
		echo -e ${RED}"DEVTYPE=$DEVTYPE"         ${NORMAL}
		echo -e ${RED}"DEVPATH=$DEVPATH"         ${NORMAL}
		echo -e ${RED}"SUBSYSTEM=$SUBSYSTEM"     ${NORMAL}
		echo -e ${RED}"SEQNUM=$SEQNUM"           ${NORMAL}
		echo -e ${RED}"UDEVD_EVENT=$UDEVD_EVENT" ${NORMAL}
	} > ${CONSOLE}
}
################################################################################
# new_name orgin_name
add_disk ()
{
	local NEW_NAME=$1
	local ORG_NAME=$2

	if [ -e ${ORG_NAME} ]; then
		cd /dev/; ln -sf ${ORG_NAME} ${NEW_NAME}; cd -
		echo "${NEW_NAME} -> ${ORG_NAME}" > ${CONSOLE}
	fi
}
################################################################################
# new_name
remove_disk ()
{
	local NEW_NAME=$1

	if [ -L /dev/${NEW_NAME} ]; then
		rm -f /dev/${NEW_NAME}
	fi
}
################################################################################
hotplug_usb ()
{
	if [ ss"${SUBSYSTEM}" != ss"block" ]; then
		return 1
	fi

	if [ ss"$(echo ${DEVPATH} | grep -r "usb")" = ss"" ]; then
		return 1
	fi

	local TMP=$(echo ${DEVPATH:36} | sed -e 's/\/host.*//')
	local NEW_NAME=${PREFIX}$(echo ${TMP##*/} | sed -e 's/[-.:]//g')
	local PART_INDEX=`echo $DEVNAME | sed -e 's/\/dev\/sd.//'`
	test -z "${PART_INDEX}" || NEW_NAME=${NEW_NAME}p${PART_INDEX}

	case "${ACTION}" in
	"add"    )
		add_disk "${NEW_NAME}" "${DEVNAME}"
	;;
	"remove" )
		remove_disk "${NEW_NAME}"
	;;
	* )
		echo "ACTION:${ACTION}" > ${CONSOLE}
	;;
	esac

	return 0
}

hotplug_sata ()
{
	if [ ss"${SUBSYSTEM}" != ss"block" ]; then
		return 1
	fi

	if [ ss"$(echo ${DEVPATH} | grep -r "ata")" = ss"" ]; then
		return 1
	fi

	local TMP=$(echo ${TMP} | sed -e 's/\/block.*//g')
	TMP=$(echo ${TMP} | sed -e 's/\/devices.*\///g')
	local NEW_NAME=${PREFIX}$(echo ${TMP} | sed -e 's/[:]//g')
	local PART_INDEX=`echo $DEVNAME | sed -e 's/\/dev\/sd.//'`
	test -z "${PART_INDEX}" || NEW_NAME=${NEW_NAME}p${PART_INDEX}

	case "${ACTION}" in
	"add"    )
		add_disk "${NEW_NAME}" "${DEVNAME}"
	;;
	"remove" )
		remove_disk "${NEW_NAME}"
	;;
	* )
		echo "ACTION:${ACTION}" > ${CONSOLE}
	;;
	esac

	return 0
}
################################################################################
#show_env
hotplug_usb
hotplug_sata