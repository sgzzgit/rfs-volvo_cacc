#!/bin/sh

slay  -f -Q -s TERM long_trk
slay  -f -Q -s TERM jbussend
slay  -f -Q -s TERM trk_wr
slay  -f -Q -s TERM rdj1939
slay  -f -Q -s TERM gpsdb
slay  -f -Q -s TERM can_man
slay  -f -Q -s TERM trk_cr
slay  -f -Q -s TERM db_slv

DATESTR=`date +%m%d%Y_%H%M%S`
echo $DATESTR >/big/data/last_start_timestamp.txt
/home/truckcontrol/test/db_slv -Q -S `hostname` &
sleep 2
/home/truckcontrol/test/trk_cr -t 1000 2>long_input$DATESTR.dbg &
sleep 2
/home/can/drv_sja1000/qnx/can_man -n /dev/can1 -s 250 -i 10 -p 0xda000 -e 1 &
sleep 2
/home/can/drv_sja1000/qnx/can_man -n /dev/can2 -s 250 -i 7 -p 0xda200 -e 1 &
