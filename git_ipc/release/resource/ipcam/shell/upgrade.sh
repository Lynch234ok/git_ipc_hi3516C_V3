#!/bin/sh

UPDATE_BASE="/tmp/upgrade/"
UPDATE_DIR="/tmp/upgrade/dev"
file=$1
#dd if=/tmp/firmware.rom of=/dev/mtdblock4 bs=1k skip=offset(K) count=size(K)

block1=$2
offset1=$3
size1=$4
block2=$5
offset2=$6
size2=$7
block3=$8
offset3=$9
size3=$10
block4=$11
offset4=$12
size4=$13

echo "file ${file}"
echo "block1 ${block1} ${offset1}  ${size1}"
echo "block2 ${block2} ${offset2}  ${size2}"
echo "block3 ${block3} ${offset3}  ${size3}"
echo "block4 ${block4} ${offset4}  ${size4}"

dd if\=${file} of\=${block1} bs\=1k skip\=${offset1} count\=${size1}
echo -n "60" > "${UPDATE_BASE}/rate"

dd if\=${file} of\=${block2} bs\=1k skip\=${offset2} count\=${size2}
echo -n "70" > "${UPDATE_BASE}/rate"

dd if\=${file} of\=${block3} bs\=1k skip\=${offset3} count\=${size3}
echo -n "80" > "${UPDATE_BASE}/rate"

dd if\=${file} of\=${block4} bs\=1k skip\=${offset4} count\=${size4}
echo -n "100" > "${UPDATE_BASE}/rate"



