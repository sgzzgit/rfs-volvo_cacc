#!/bin/sh
# Runs a series of tests and accumulates interarrival time and dropped
# message statistics at the receiver. One system remains the sender and
# the other the receiver throught the tests.
#
# To be run on the sending system; relies on ssh keys having been set up
# between the two systems, so that processes may be started on the remote
# system. Assumes the CAN driver is already running 

REMOTE_SYSTEM=192.168.1.102
SAVE_DIR=/dev/shmem
CLIENT_DIR=/home/capath/can/client/path_can/qnx
CAN_ID=101
CAN_RX=can_rx3
LOCAL_CHANNEL=1
REMOTE_CHANNEL=1
for HZ in 10 100 1000 10000
do 
	ssh $REMOTE_SYSTEM "$CLIENT_DIR/$CAN_RX -p /dev/can$REMOTE_CHANNEL  >$SAVE_DIR/can_rx2.$HZ &" &
	sleep 1
# timestep interval for message send, inverse of Hz 
	INTERVAL=`echo $HZ | awk '{printf("%.6f", 1.0/$1)}'`
# runs the test for COUNT timesteps (1 minute)
	COUNT=`echo $HZ | awk '{printf("%d", 60*$1)}'`
	echo $HZ $INTERVAL $COUNT
	$CLIENT_DIR/can_tx -p /dev/can$LOCAL_CHANNEL -i $CAN_ID -e 1 -t $INTERVAL -n $COUNT 
	ssh $REMOTE_SYSTEM "/bin/slay $CAN_RX"
	sleep 1
done
