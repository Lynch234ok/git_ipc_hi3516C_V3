#!/bin/sh

arg_count=$#
#echo "The number of parameters: $arg_count"

arg1=$1
arg2=$2
arg3=$3

#echo "arg1 = $arg1"
#echo "arg2 = $arg2"
#echo "arg3 = $arg3"
#echo "==========================="

# {"SystemRunningTime":"0000-0:39:12", "CPU": {"usr": 13.3,"sys": 20.0} , "VmRSS": 8760, "VmSize": 50208, "freeMemory": 1168, "PID": 1270, "IOTDaemonPID": 1269, "wifiSignal": 0}

# 运行时间
systemTime=`cat /proc/uptime | awk -F. '{mon=$1/2592000; day=$1/86400; hour=($1%86400)/3600; minute=($1%3600)/60; second=$1%60; printf("\"SystemRunningTime\": \"%02d%02d-%d:%d:%d\"", mon, day, hour, minute, second)}'`

cpuInfo=`top -n 1 | grep '^CPU' |awk '{printf("\"CPU\": { \"usr\": %5.2f, \"sys\": %5.2f }", $2, $4)}'`	        # cpu(usr/sys)

fPID=`ps | grep '/usr/share/ipcam/app'| grep '/media/custom/web'| grep -v '{JPROCESS_LOOP}' |awk '{printf("%s",$1)}'`   # PID，ps
PID="\"PID\": "$fPID""

physical_memory=`cat /proc/$fPID/status | grep 'VmRSS' | awk '{printf("\"VmRSS\": %s", $2)}'`   # 物理内存

virtual_Memory=`cat /proc/$fPID/status | grep 'VmSize' | awk '{printf("\"VmSize\": %s", $2)}'`  # 虚拟内存

free_memory=`cat /proc/meminfo | grep 'MemFree' | awk '{printf("\"freeMemory\": %s", $2)}'`	# 剩余内存

iotDaemon_PID=`ps | grep 'iot.Daemon'| grep 'IOTDaemon'| awk '{printf("\"IOTDaemonPID\": %s",$1)}'` # iotDaemon PID，

if [ -d "/proc/net/rtl8188fu/" ];then								# wifi信号值
	wifi_signal=`cat /proc/net/rtl8188fu/wlan0/rx_signal | grep 'signal_strength' | awk -F":" '{printf("\"wifiSignal\": %d", $2)}'`	        # eus
else
	wifi_signal=`cat /proc/net/rtl8188eu/wlan0/rx_signal | grep 'signal_strength' | awk -F":" '{printf("\"wifiSignal\": %d", $2)}'`		# ftv
fi



if [ 0 -eq $arg_count ];then		 # 无参数，显示所有信息 	
	echo "{ "$systemTime", "$cpuInfo", "$physical_memory", "$virtual_Memory", "$free_memory", "$PID", "$iotDaemon_PID", "$wifi_signal" }"
else				         # 有参数，根据参数显示信息
	if [ 3 -ge $arg_count ];then     # if(3 > arg_count)
		#for(( i = 0; i < $arg_count; i++ ))
		for i in `seq $arg_count`		
		do

#-----------------------------------------------------------------------
			if [ 1 -eq $i ];then				
				arg=$1
			fi

			if [ 2 -eq $i ];then				
				arg=$2  
			fi

			if [ 3 -eq $i ];then				
				arg=$3
			fi
#-----------------------------------------------------------------------

			#echo "in for: $i"
			#echo "$arg <<"
			
			if [ $arg == "systime" ];then				
				INFO=""$INFO""$systemTime", " 
			fi
			
			if [ $arg == "cpuinfo" ];then
				INFO=""$INFO""$cpuInfo", " 
			fi
			
			if [ $arg == "vmsize" ];then
				INFO=""$INFO""$virtual_Memory", "
			fi
			
			if [ $arg == "vmrss" ];then
				INFO=""$INFO""$physical_memory", "
			fi	
			
			if [ $arg == "freemem" ];then
				INFO=""$INFO""$free_memory", "
			fi	
			
			if [ $arg == "iotpid" ];then
				INFO=""$INFO""$iotDaemon_PID", "
			fi

			if [ $arg == "pid" ];then
				INFO=""$INFO""$PID", "
			fi

			if [ $arg == "wifi" ];then
				INFO=""$INFO""$wifi_signal", "
			fi

		done
		
		echo -e "{ \c"
		echo -e "${INFO%,*}\c"
		echo " }"	
	else
		echo "erro: The number of parameters is greater than 3!"
	fi
fi






