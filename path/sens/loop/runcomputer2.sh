#!/bin/sh


echo kill all process
/home/vision/VehicleTracker/VirtualLoop/kill.sh&

sleep 5

echo set_tty_for_raw
/home/path/sens/loop/set_tty_for_raw

#echo synconize the two computer
/home/path/clt/rtperf/lnx/clock_set&

echo run database
/home/path/db/lnx/db_slv & 

sleep 1
echo run loop2db
/home/path/sens/loop/loop2db &

#sleep 1
echo rcvloop......
/home/vision/VehicleTracker/VirtualLoop/rcvloop &

sleep 5
echo wrfile......
/home/vision/VehicleTracker/VirtualLoop/wrfile


