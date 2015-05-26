#!/bin/sh
# Runs a series of tests and accumulates interarrival time and dropped
# message statistics at two receivers on the same system. One system
# runs the script and starts an additional transmit process on a remote
# system, and two receive processes on another remote system. 
#
# Relies on ssh keys having been set up on all systems, so that processes
# may be started and stopped remotely. Waits for transmit process on remote
# system to end before killing receive processes on the other remote system.
# So timing measurement will be inaccurate if transmission start, stop or 
# duration is significantly different on the two transmitting systems. 
#
# Assumes the CAN driver is already running on all systems. 

SAVE_DIR=/big/data/can
CLIENT_DIR=/home/capath/can/client/path_can/qnx
REMOTE_RX=192.168.1.103
REMOTE_TX=192.168.1.102
CAN_LOCAL_ID=101
CAN_REMOTE_ID=102
CAN_RX=can_rx2
LOCAL_TX_CHANNEL=3
REMOTE_TX_CHANNEL=2
RX1_CHANNEL=1
RX2_CHANNEL=2
EXTENDED=1
for HZ in 10 20 50 100 200 500 1000 2000 2500 5000 10000
do 
	ssh $REMOTE_RX "$CLIENT_DIR/$CAN_RX -p /dev/can$RX1_CHANNEL  >$SAVE_DIR/p$RX1_CHANNEL$CAN_RX.$HZ &" &
	ssh $REMOTE_RX "$CLIENT_DIR/$CAN_RX -p /dev/can$RX2_CHANNEL  >$SAVE_DIR/p$RX2_CHANNEL$CAN_RX.$HZ &" &
	sleep 1
# timestep interval for message send, inverse of Hz 
	INTERVAL=`echo $HZ | awk '{printf("%.6f", 1.0/$1)}'`
# runs the test for COUNT timesteps (1 minute)
	COUNT=`echo $HZ | awk '{printf("%d", 60*$1)}'`
	echo $HZ $INTERVAL $COUNT
	$CLIENT_DIR/can_tx -p /dev/can$LOCAL_TX_CHANNEL -i $CAN_LOCAL_ID -e $EXTENDED -t $INTERVAL -n $COUNT & 
	ssh $REMOTE_TX "$CLIENT_DIR/can_tx -p /dev/can$REMOTE_TX_CHANNEL -i $CAN_REMOTE_ID -e $EXTENDED -t $INTERVAL -n $COUNT"  
	ssh $REMOTE_RX "/bin/slay $CAN_RX"
	sleep 1
done
