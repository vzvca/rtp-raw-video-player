#! /bin/sh

SRC=239.192.77.10
SRCP=5004
IP=127.0.0.1
IF=lo

socat UDP4-RECV:$SRCP,bind=$SRC,ip-add-membership=$SRC:$IF,reuseaddr - > /tmp/xxx
