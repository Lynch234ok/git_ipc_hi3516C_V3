
#!/bin/sh

#modprobe hiether
#sleep 3

#############################################################################################
# Linux Configuration
#############################################################################################
ETC_LINK_DIR=/usr/local/etc
ETC_LINK_RESOLV=$ETC_LINK_DIR/resolv.conf

mkdir -p $ETC_LINK_DIR
touch $ETC_LINK_RESOLV
echo "nameserver 8.8.8.8" >> $ETC_LINK_RESOLV
echo "nameserver 8.8.4.4" >> $ETC_LINK_RESOLV
echo "nameserver 4.4.4.4" >> $ETC_LINK_RESOLV
echo "nameserver 168.95.1.1" >> $ETC_LINK_RESOLV
#sleep 1

#############################################################################################
# Frank's Debug Network Environment
#############################################################################################
ifconfig eth0 192.168.1.168 up
ifconfig eth0:1 192.168.250.250 up
ifconfig eth0:2 192.168.168.168 up
ifconfig lo 127.0.0.1 up

#############################################################################################
# Linux Telnet Service
#############################################################################################
DEBUG_INI_PATH="/media/tf/debug.ini"
MOUNT_DIR=/media/tf
LOG_REDIRECT=0
# check mount status
if [ -r $MOUNT_DIR ];then
	echo "mount dir('$MOUNT_DIR') is exist now!"
else
	echo "mount dir('$MOUNT_DIR') isn't exist now!"
	mkdir $MOUNT_DIR
	mount /dev/mmcblk0p1  /media/tf
	#sleep 2
fi
if [ -f "$DEBUG_INI_PATH" ];then
	telnetEnable=`cat "$DEBUG_INI_PATH" | grep "telnet" | sed 's/telnet=//g'`
	if [ "$telnetEnable" == 1 ];then
		telnetd &
	fi
	debugEnable=`cat "$DEBUG_INI_PATH" | grep "debug" | sed 's/debug=//g'`
	echo "debug flag is $debugEnable"
	if [ $debugEnable == 1 ];then
		echo "open debug mode"
		if [ -e $MOUNT_DIR/ipcam_app ];then
			echo "mount bind ipcam_app"
			mount --bind $MOUNT_DIR/ipcam_app /usr/share/ipcam/app
			ulimit -c unlimited
			ulimit -a
			echo "$MOUNT_DIR/%e_%t.core" > /proc/sys/kernel/core_pattern
		fi
		touch /tmp/tf_loging
		LOG_REDIRECT=1
	fi
else
    echo "no debug ini file"
fi

#############################################################################################
# Application Environment
#############################################################################################
HOMEDIR=/usr/share/ipcam

RESDIR=$HOMEDIR/resource
FLASHMAP=$HOMEDIR/flashmap.ini
SYSCONF=/dev/mtdblock3
APPEXEC=$HOMEDIR/app
APPAGENT=$HOMEDIR/agent
APPITTB=$HOMEDIR/ittb.tar.gz
NETSDKDIR_DEF=$RESDIR/netsdk
SHELLDIR=$HOMEDIR/shell

CONFMTD=/dev/mtdblock6
CUSTOMMTD=/dev/mtdblock7
CONFDIR=/media/conf
CUSTOMDIR=/media/custom
FONTDIR=$CUSTOMDIR/font
WEBDIR=$CUSTOMDIR/web
JFFS2IMG=$CUSTOMDIR/custom.jffs2
CONFSHELLDIR=$CONFDIR/shell

HOOK_STARTUP=$CONFSHELLDIR/startup.sh
HOOK_STOP=$CONFSHELLDIR/stop.sh

#############################################################################################

  #############################################################################################
# Frank's Filesystem
#############################################################################################

# make mount point
mkdir /media/conf
mkdir /media/custom
mkdir /tmp/upgrade
# mount my configuration rw filesystem
#cp $CUSTOMMTD /tmp/custom.fs
mount -t squashfs $CUSTOMMTD $CUSTOMDIR
mount -t jffs2 $CONFMTD $CONFDIR
if [ "$?" != "0" ]; then
	dd if=$JFFS2IMG of=$CONFMTD
	mount -t jffs2 $CONFMTD $CONFDIR
fi

# mount my custom rw filesystem

# reset the resource environment
NETSDKDIR=$CONFDIR/netsdk

if [ ! -d "$NETSDKDIR" ]; then
	echo 'Building System Configuration...'
	#cp -af $NETSDKDIR_DEF $NETSDKDIR
fi

if [ ! -d "$CONFSHELLDIR" ]; then
	echo 'Building shell Configuration...'
	mkdir $CONFSHELLDIR
	cp -af $SHELLDIR/landhcp.script $CONFSHELLDIR
	cp -af $SHELLDIR/startup.sh $CONFSHELLDIR
	cp -af $SHELLDIR/stop.sh $CONFSHELLDIR
	cp -af $SHELLDIR/upgrade.sh $CONFSHELLDIR
	echo '#!/bin/sh' > $HOOK_STARTUP 
	echo 'exit 0' > $HOOK_STARTUP 
	chmod +x $HOOK_STARTUP
fi

#############################################
# Startup Application
#############################################

# Aplication Link
ln -s $APPEXEC /usr/local/sbin/READ_INFO
ln -s $APPEXEC /usr/local/sbin/RSERIAL_NUM
ln -s $APPEXEC /usr/local/sbin/WSERIAL_NUM
ln -s $APPEXEC /usr/local/sbin/RRTC
ln -s $APPEXEC /usr/local/sbin/WRTC
mkdir /tmp/run
mkdir /tmp/lib

echo "Do You Want to Run Application ? (y/n)"
read -t 1 -n 1 char
if [ "$char" == "n" ]; then
	echo "Application Cancel!"
elif [ "$char" == "f" ]; then

	# Linux core
	echo "Frank Debug Mode!"
	ulimit -c unlimited
	echo "1" > /proc/sys/kernel/core_uses_pid
	echo "/tmp/%t.%e.core" > /proc/sys/kernel/core_pattern
	echo "3000000" > /proc/sys/net/core/rmem_default
	echo "3000000" > /proc/sys/net/core/rmem_max
	
	# Network debug environment
	ifconfig eth0 192.168.2.45 up
	sleep 1
	mount -t nfs -o nolock 192.168.2.41:/root/nfs /root/nfs 
	export PATH=$PATH:"/root/nfs/bin"
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"/root/nfs/lib"
	rmmod watchdog

elif [ $LOG_REDIRECT == 1 ]; then
	echo "enter debug mode!"
	$HOOK_STARTUP
	#$HOMEDIR/startup.sh
	cat /proc/kmsg >> $MOUNT_DIR/kmsg.log &
	if [ "0" == "$?" ]; then
		# Loop to Startup Application
		tmp=`/usr/local/sbin/RSERIAL_NUM  | grep "serial number"`
		SN=`echo "$tmp" | sed 's/serial number://g'`
		P2P_DEBUG_PATH="/media/tf/p2p_debug/"
		P2P_DEBUG_SCRIPT="$P2P_DEBUG_PATH""/p2p_debug.sh"
		if [ -f "$P2P_DEBUG_SCRIPT" ];then
			$P2P_DEBUG_SCRIPT "$SN" &
		else
			echo "no p2p debug path"
			$HOMEDIR/shell/daemon_start.sh "$SN" "$LOG_REDIRECT" &
		fi
		while [ 1 ]
		do
			cd `dirname $PWD`
			# Startup with Agent
			#$APPAGENT $APPEXEC -w $WEBDIR -f $FONTDIR -m $FLASHMAP -d $DEFCONF -s $SYSCONF -c $SYSCONF2
			# Startup without Agent
			CURTIME=`date +%y%m%d_%H%M%S`
			LOG_FILE=ipc_$CURTIME.log
			echo $LOG_FILE
			$APPEXEC -w $WEBDIR -f $FONTDIR -m $FLASHMAP -s $SYSCONF -d $NETSDKDIR_DEF -c $NETSDKDIR >> "$MOUNT_DIR/$LOG_FILE"
		done
	fi

else
#	insmod /root/bsp/i2cm.ko
#	insmod /root/bsp/rtc.ko
#	insmod /root/bsp/watchdog.ko

	# RTC Sync
	#hwclock -r

	$HOOK_STARTUP
	#$HOMEDIR/startup.sh
	if [ "0" == "$?" ]; then
		# Loop to Startup Application
		tmp=`/usr/local/sbin/RSERIAL_NUM  | grep "serial number"`
		SN=`echo "$tmp" | sed 's/serial number://g'`
		P2P_DEBUG_PATH="/media/tf/p2p_debug/"
		P2P_DEBUG_SCRIPT="$P2P_DEBUG_PATH""/p2p_debug.sh"
		if [ -f "$P2P_DEBUG_SCRIPT" ];then
			$P2P_DEBUG_SCRIPT "$SN" &
		else
			echo "no p2p debug path"
			$HOMEDIR/shell/daemon_start.sh "$SN" "$LOG_REDIRECT" &
		fi
		while [ 1 ]
		do
			cd `dirname $PWD`
			# Startup with Agent
			#$APPAGENT $APPEXEC -w $WEBDIR -f $FONTDIR -m $FLASHMAP -d $DEFCONF -s $SYSCONF -c $SYSCONF2
			# Startup without Agent
			$APPEXEC -w $WEBDIR -f $FONTDIR -m $FLASHMAP -s $SYSCONF -d $NETSDKDIR_DEF -c $NETSDKDIR
		done
	fi
fi


