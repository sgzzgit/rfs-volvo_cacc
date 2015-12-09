#!/bin/sh

/home/truckcontrol/long_ctl/src/avcs/slay_can.sh

IPADDR=`ifconfig | grep 192.168.0 | awk '{print $2}'`

echo $IPADDR | grep 201 
if [[ $? -eq 0 ]]
then
	ITRIADDR=192.168.0.75
	TRUCK=Blue
fi

echo $IPADDR | grep 202 
if [[ $? -eq 0 ]]
then
	ITRIADDR=192.168.0.74
	TRUCK=Gold
fi

echo $IPADDR | grep 203
if [[ $? -eq 0 ]]
then
	ITRIADDR=192.168.0.76
	TRUCK=Silvr
fi

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
/home/truckcontrol/long_ctl/src/vehcomm/qnx/veh_snd -v -d -i 100 -A $IPADDR -a $ITRIADDR -u 15041 -t $TRUCK >veh_snd.dbg &
sleep 1
/home/truckcontrol/long_ctl/src/vehcomm/qnx/veh_rcv -v -A $IPADDR -a $ITRIADDR -u 15042 -t $TRUCK >veh_rcv.dbg &
