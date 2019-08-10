#!/bin/sh
################################################################################
#    Create By Czy
################################################################################

#
# usb device event:
#
# ACTION=add 
# DEVNAME=/dev/usbdev1.13 
# DEVTYPE= 
# DEVPATH=/devices/platform/hiusb-ehci.0/usb1/1-2/1-2.4/usb_device/usbdev1.13 
# SUBSYSTEM=usb_device 
# SEQNUM=555 
# UDEVD_EVENT=1 
#

################################################################################
USB_PREFIX=usbdev
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
add_usb ()
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
remove_usb ()
{
	local NEW_NAME=${1}
	if [ -L /dev/${NEW_NAME} ]; then
		rm -f /dev/${NEW_NAME}
	fi
}
################################################################################
hotplug_usb ()
{
	if [ ss"$SUBSYSTEM" != ss"usb_device" ]; then
		return 1
	fi

	if [ ss"$(echo $DEVPATH | grep -r "usb")" = ss"" ]; then
		return 1
	fi

	TMP=$(echo ${DEVPATH:36} | sed -e 's/\/usb_device.*//')
	NEW_NAME=${USB_PREFIX}$(echo ${TMP##*/} | sed -e 's/[-.usbdev]//g')

	case "$ACTION" in
	"add"    )
		add_usb "${NEW_NAME}" "${DEVNAME}"
	;;
	"remove" )
		remove_usb "${NEW_NAME}"
	;;
	* )
		echo "Not recognise ACTION:${ACTION}" > ${CONSOLE}
	;;
	esac

	return 0
}
################################################################################

#show_env
hotplug_usb
