#!/bin/sh

CTR=8
while [[ $CTR -ge 0 ]]
do
	/home/path/sens/gps/examples/qnx/gpssetdate -v </dev/ser1 &
	sleep 2
	slay gpssetdate
	CTR=$(($CTR-1))
done
