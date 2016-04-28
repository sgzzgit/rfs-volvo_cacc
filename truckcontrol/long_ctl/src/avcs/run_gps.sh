#!/bin/sh

slay  -f -Q -s TERM jbussendGPS
slay  -f -Q -s TERM gpsdb
slay  -f -Q -s TERM can_man
slay  -f -Q -s TERM trk_cr
slay  -f -Q -s TERM db_slv

GPSDBNUM=202

/home/truckcontrol/test/db_slv -Q -S `hostname` &
sleep 2
/home/truckcontrol/test/trk_cr -t 1000 2>long_input.dbg &
sleep 2
/home/can/drv_sja1000/qnx/can_man -n /dev/can1 -s 250 -i 10 -p 0xd8000 -e 1 &
sleep 2
/home/can/drv_sja1000/qnx/can_man -n /dev/can2 -s 250 -i 7 -p 0xd8100 -e 1 &
sleep 2
/home/truckcontrol/test/gpsdb -n $GPSDBNUM -d1 </dev/ser1 &
sleep 2
/home/truckcontrol/long_ctl/src/avcs/qnx/jbussendGPS  -n $GPSDBNUM -c -g /dev/can2 &
