#!/bin/sh
# If using the Steinhoff driver, use 
# CAN_CLIENT_DIR=/home/capath/can/client/steinhoff/qnx
# instead
CAN_CLIENT_DIR=/home/capath/can/client/path_can/qnx
VAANUM=$1
HZ=$2
REPS=$3
CH=$4
MS=`echo $HZ | awk '{printf("%.3f", 1.0/$HZ)}'`
echo $HZ $MS
ssh dickey@192.168.1.$VAANUM "$CAN_CLIENT_DIR/can_tx -p /dev/can$CH -i $HZ -e 1 -t $MS -n $REPS" 
