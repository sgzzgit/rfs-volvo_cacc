#!/bin/sh

/home/truckcontrol/long_ctl/src/avcs/slay_can.sh

DATESTR=`date +%m%d%Y_%H%M%S`
echo $DATESTR >/big/data/last_start_timestamp.txt
/home/truckcontrol/test/db_slv -Q -S `hostname` &
sleep 1
/home/truckcontrol/test/trk_cr -t 1000 2>long_input$DATESTR.dbg &
sleep 1
/home/can/drv_sja1000/qnx/can_man -n /dev/can1 -s 250 -i 10 -p 0xda000 -e 1 &
sleep 1
/home/can/drv_sja1000/qnx/can_man -n /dev/can2 -s 250 -i 7 -p 0xda200 -e 1 &
sleep 1
/home/truckcontrol/long_ctl/src/vehcomm/qnx/veh_snd -v -i 100 -A 192.168.0.201 -a 192.168.0.31 -u 15042 -t Blue 
