
while [ 1 ]
do
	wget "http://admin:@192.168.2.45/NetSDK/Video/motionDetection/channel/1/status" -o log -O status.json
	cat status.json
	echo ""
done
