#!/bin/bash

function error_strap() {
	echo "$1" 1>&2
	exit 1
}

function extract_rootfs()
{
	rootfsName="$1"
	rootfs="$rootfsName"".tar.gz"
	rootfsPath="../release/backup/"
	releasePath="../release/"
	rm "$releasePath""$rootfsName" -r
	tar xzf "$rootfsPath""$rootfs" -C "$releasePath"

}

function compile_steps() {
	local ret
	local err

	local model="$1.ini"
	
	echo $model
	cp configs/$model config.ini

	rootfsName=`cat config.ini | grep "MODEL_ROOTFS := " | sed 's/MODEL_ROOTFS := //g' | sed 's/\r//g'`
	extract_rootfs "$rootfsName"

	make clean
	make sdk || error_strap "sdk error:"$model
	make image || error_strap "image error:"$model
}

#compile_steps hi3516a_inc_config
#compile_steps hi3516c_v2_inc_config
#compile_steps hi3516d_inc_config
#compile_steps hi3516d_ws_inc_config
#compile_steps hi3518a_inc_config
#compile_steps hi3518a_wsc_inc_config
#compile_steps hi3518c_ws_inc_config
#compile_steps hi3518e_inc_config
#compile_steps hi3518e_v2_inc_config
#compile_steps hi3518e_v2_ws_inc_config
#compile_steps hi3518e_w_inc_config
#compile_steps hi3518e_ws_inc_config
#compile_steps hi3518e_wsc_inc_config
#compile_steps m388c1g_180m_inc_config

# P2/P3/P4/C2/C3/C4/P2-nel
compile_steps P3_hi3518e_v2_inc_config

# P6
#compile_steps P6_hi3516d_inc_config

# P2-720
#compile_steps P2_720_hi3518e_v2_inc_config

exit

