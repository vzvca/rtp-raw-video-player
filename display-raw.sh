#!/usr/bin/env sh
#IPINTF=enx000ec6c9813b
#IPINTF=eth0
#IPINTF=enp3s0.10
IPINTF=wlp2s0
ENCODING_NAME=RAW
COLORIMETRY=BT709-2

if [ ${SESSION_NAME} = ${ID_AVD_NATIVE} ]; then
	IPMULTICAST=239.192.72.16
	WIDTH=720
	HEIGHT=576
elif [ ${SESSION_NAME} = ${ID_AVD_VIGNETTE} ]; then
	IPMULTICAST=239.192.73.16
	WIDTH=424
	HEIGHT=340
else
	echo "unknown cam";
	exit 1;
fi	

gst-launch-1.0 udpsrc port=5004 multicast-group=239.192.77.10 multicast-iface=lo ! application/x-rtp, media=video, clock-rate=90000, encoding-name=${ENCODING_NAME}, sampling=YCbCr-4:2:0, width="(string)720", height="(string)576", payload=97, depth='(string)8', colorimetry=BT709-2 ! queue ! rtpvrawdepay ! videoconvert ! autovideosink sync=false

