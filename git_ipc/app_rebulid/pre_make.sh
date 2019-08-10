#/bin/bash

echo make ipc ${1}
CONFIGS_PATH="./configs"
CONFFIG=`find $CONFIGS_PATH -type f -name '*.ini'|xargs grep ${1} -l`

make clean
echo $CONFFIG
cp $CONFFIG config.ini
make sdk
make
if [ ${2} == "image" ]
then
	make image
fi
