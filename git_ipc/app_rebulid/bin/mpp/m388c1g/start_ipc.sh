#!/bin/sh
mkdir -p /tmp/venc/c0/
export PATH=${PATH}:./
export LD_LIBRARY_PATH=./lib
./tinyvenc -D -c Resource/4/ar0330_1536x1536_ch4.cfg -a Resource/Autoscene/autoscene_conf.cfg 
./rtsps -c stream_server_config.ini &
