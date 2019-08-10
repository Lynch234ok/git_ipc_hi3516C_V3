#!/bin/sh

insmod vpl_vic.ko abAEEn=1,1 abIrisEn=0,0 abAFEn=0,0 abAWBEn=1,1 gdwBusFreq=200000000 gdwSignalWaitTime=4000

# sensor.ko


sensorType=`$(pwd)/../sensor_type.sh`

if [ $sensorType == "SC2045" ]; then
    echo "insmod SC2045"
    insmod SC2045.ko dwSignalPort=0 adwVideoWidth=1920,0 adwVideoHeight=1080,0 adwIICBusNum=1,0 abSysPLLUsed=1,0

elif [ $sensorType == "AR0330" ]; then
    echo "insmod AR0330"
    insmod AR0330.ko dwSignalPort=1 adwVideoWidth=2304,2304 adwVideoHeight=1536,1536 adwIICBusNum=1,1 abEnMipi=0,1 abEnData10bit=0,1 abSysPLLUsed=1,1 adwFPS=20,20 abAntiBlooming=0,0
    #insmod AR0330.ko dwSignalPort=1 adwVideoWidth=2304,2304 adwVideoHeight=1536,1536 adwIICBusNum=1,1 adwDevI2CID=0x20,0x20 abEnMipi=0,1 abEnData10bit=0,1 abSysPLLUsed=1,1 adwFPS=20,20 abAntiBlooming=0,0
else
    echo "War:Don't read sensor!"

fi

#insmod AR0330.ko dwSignalPort=1 adwVideoWidth=2304,2304 adwVideoHeight=1536,1536 adwIICBusNum=1,1 abEnMipi=0,1 abEnData10bit=0,1 abSysPLLUsed=1,1 adwFPS=20,20 abAntiBlooming=0,0
## insmod AR0330.ko dwSignalPort=1 adwVideoWidth=1536,1536 adwVideoHeight=1536,1536 adwIICBusNum=1,1 abEnMipi=0,1 abEnData10bit=0,1 abSysPLLUsed=1,1

insmod AutoExposure.ko
insmod AutoWhiteBalance.ko
