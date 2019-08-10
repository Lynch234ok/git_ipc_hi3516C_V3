#!/bin/sh
mkdir -p /tmp/venc/c0/
export PATH=${PATH}:./
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib

sensorType=`$(pwd)/sensor_type.sh`

if [ $sensorType == "SC2045" ]; then
	./mstrm_venc_2stream_2045 -D -c Resource_2045/0/sc2045_1920x1080_ch0.cfg -a Resource_2045/Autoscene/autoscene_conf.cfg -i ivs_default_setting.ini
	sleep 5
    ./FEC_parameters/180p.sh
elif [ $sensorType == "AR0330" ]; then
	./mstrm_venc_2stream_0330 -D -c Resource_0330/4/ar0330_1600x1536_ch4.cfg -a Resource_0330/Autoscene/autoscene_conf.cfg -i ivs_default_setting.ini

else
    echo "War:Don't read sensor!"

fi

