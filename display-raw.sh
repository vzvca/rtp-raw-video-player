#!/usr/bin/env sh
#IPINTF=enx000ec6c9813b
IPINTF=lo
#IPINTF=enp3s0.10
#IPINTF=wlp2s0
ENCODING_NAME=RAW
COLORIMETRY=BT709-2

IPMULTICAST=239.192.77.10
WIDTH=720
HEIGHT=576

gst-launch-1.0 udpsrc port=5004 multicast-group=${IPMULTICAST} multicast-iface=lo ! application/x-rtp, media=video, clock-rate=90000, encoding-name=${ENCODING_NAME}, sampling=YCbCr-4:2:0, width="(string)${WIDTH}", height="(string)${HEIGHT}", payload=96, depth='(string)8', colorimetry=BT709-2 ! queue ! rtpvrawdepay ! videoconvert ! autovideosink sync=false

