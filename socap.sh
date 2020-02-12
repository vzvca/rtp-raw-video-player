#! /bin/sh

SRC=239.192.77.10
SRCP=5004
IP=10.62.73.3
IF=eth0.10

socat UDP4-RECV:$SRCP,bind=$SRC,ip-add-membership=$SRC:$IF,reuseaddr - > /dev/null
