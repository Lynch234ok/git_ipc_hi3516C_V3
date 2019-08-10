#!/bin/sh
file="/media/conf/actual_stream"
mkdir -p /tmp/venc/c0/
export PATH=${PATH}:./
export LD_LIBRARY_PATH=./lib
#./tinyvenc -D -c Resource/4/ar0330_1536x1536_ch4.cfg -a Resource/Autoscene/autoscene_conf.cfg
if [ ! -f "$file" ]; then
	echo "[start_ipc_m.sh]: 2 stream"
	./mstrm_venc_2stream -D -c Resource/4/ar0330_1536x1536_ch4.cfg -a Resource/Autoscene/autoscene_conf.cfg
else
	echo "[start_ipc_m.sh]: 4 stream"
	./mstrm_venc -D -c Resource/4/ar0330_1536x1536_ch4.cfg -a Resource/Autoscene/autoscene_conf.cfg
fi 

#./mstrm_venc -c Resource/4/ar0330_1536x1536_ch4.cfg -a Resource/Autoscene/autoscene_conf.cfg
#./rtsps -c stream_server_config.ini &
#./ipcam_app




