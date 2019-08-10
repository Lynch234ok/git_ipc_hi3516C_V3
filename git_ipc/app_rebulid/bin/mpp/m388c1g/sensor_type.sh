#!/bin/sh


sensor2045Addr=`i2cdetect  -r -y 1 | grep 30 | awk '{print $2}'`
sensor0330Addr1=`i2cdetect  -r -y 1 | grep 10 | awk '{print $2}'`
sensor0330Addr2=`i2cdetect  -r -y 1 | grep 10 | awk '{print $10}'`


if [ $sensor2045Addr != "--" ]; then
    sensorType="SC2045"

elif [ $sensor0330Addr1 != "--" ]; then
    sensorType="AR0330"

elif [ $sensor0330Addr2 != "--" ]; then
    sensorType="AR0330"

else
    sensorType="NULL"

fi

echo $sensorType
echo $sensorType > /media/conf/sensor_type

