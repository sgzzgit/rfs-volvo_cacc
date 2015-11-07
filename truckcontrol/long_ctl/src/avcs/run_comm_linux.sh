#!/bin/sh

killall veh_snd
killall trk_cr
killall db_slv
TOPDIR="/home/jspring/volvo/homedirs/home_blue192_volvo_474/home"
DATESTR=`date +%m%d%Y_%H%M%S`
echo $DATESTR >/big/data/last_start_timestamp.txt
/home/path/db/lnx/db_slv &
sleep 1
$TOPDIR/truckcontrol/test/trk_cr -t 1000 2>long_input$DATESTR.dbg &
sleep 1
$TOPDIR/truckcontrol/long_ctl/src/vehcomm/lnx/veh_snd -v -i 100 -A 192.168.0.133 -a 192.168.0.32 -u 15052 -t Blue 
