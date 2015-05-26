#!/bin/sh
# Save data from a series of can_rx/can_tx runs between VAA1 and VAA2.
# Uses the Steinhoff driver, assumes it has already been started.
#
# Identifier will be the Hz of the can_tx process
# Only one file will be saved for each direction.
#
# Currently crashes can_dual when I try to run both directions at once.
#
TCP1=3275
TCP2=3276
REPS=20

/usr/local/bin/nc -l -p $TCP1 > $TCP1.txt &
#/usr/local/bin/nc -l -p $TCP2 > $TCP2.txt &
ssh dickey@192.168.1.101 "/home/capath/can/test/can_rx -p 2 | /usr/local/bin/nc 192.168.1.103 $TCP1" &
#ssh dickey@192.168.1.102 "/home/capath/can/test/can_rx -p 2 | /usr/local/bin/nc 192.168.1.103 $TCP2" &

#the following loop causes can_dual to crash
#for HZ in 1 2 5 10 20
#do
#	MS=`echo $HZ | awk '{printf("%.3f", 1.0/$HZ)}'`
#	echo $HZ $MS
#	ssh dickey@192.168.1.101 "/home/capath/can/test/can_tx -p 2 -i $HZ -e 1 -t $MS -n $REPS" &
# Second one not in background, wait for it to finish before next
#	ssh dickey@192.168.1.102 "/home/capath/can/test/can_tx -p 2 -i $HZ -e 1 -t $MS -n $REPS" 
# Try waiting before starting a new one after old one has finished
#	sleep 5
#done 
