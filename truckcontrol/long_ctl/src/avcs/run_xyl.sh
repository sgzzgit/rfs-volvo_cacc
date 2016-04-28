#!/bin/sh

/home/truckcontrol/long_ctl/src/avcs/slay_can.sh

IPADDR=`ifconfig | grep 172.16.0 | awk '{print $2}'`

echo $IPADDR | grep 75
if [[ $? -eq 0 ]]
then
        ITRIADDR=172.16.1.75
        TRUCK=Blue
fi

echo $IPADDR | grep 74 
if [[ $? -eq 0 ]]
then
        ITRIADDR=172.16.1.74
        TRUCK=Gold
fi

echo $IPADDR | grep 76
if [[ $? -eq 0 ]]
then
        ITRIADDR=172.16.1.76
        TRUCK=Silvr
fi

DATESTR=`date +%m%d%Y_%H%M%S`
echo $DATESTR >/big/data/last_start_timestamp.txt
/home/truckcontrol/test/db_slv -Q -S `hostname` &
sleep 1
/home/truckcontrol/test/trk_cr -t 1000 2>long_input$DATESTR.dbg &
sleep 1
/home/can/drv_sja1000/qnx/can_man -n /dev/can1 -s 250 -i 10 -p 0xd8000 -e 1 &
sleep 1
/home/can/drv_sja1000/qnx/can_man -n /dev/can2 -s 500 -i 7 -p 0xd8100 -e 1 &
sleep 1
/home/truckcontrol/long_ctl/src/vehcomm/qnx/veh_snd -v -d -i 100 -A $IPADDR -a $ITRIADDR -u 15041 -t $TRUCK >veh_snd.dbg &
sleep 1
/home/truckcontrol/long_ctl/src/vehcomm/qnx/veh_rcv -v -A $IPADDR -a $ITRIADDR -u 15042 -t $TRUCK >veh_rcv.dbg &
sleep 1
/home/truckcontrol/test/gpsdb -n 202 -d1 </dev/ser1 &
sleep 1
/home/can/jbus/src/qnx/rdj1939 -v -c -f /dev/can1 >/big/data/can1$DATESTR.txt &
sleep 1
/home/can/jbus/src/qnx/rdj1939 -v -c -f /dev/can2 >/big/data/can2$DATESTR.txt &
sleep 2
#/home/truckcontrol/test/jbussend -d -c -e /dev/can2 -b /dev/can2 >/home/truckcontrol/test/jbussend_$DATESTR.dbg &
/home/truckcontrol/test/jbussend -c -e /dev/can1 -b /dev/can1 >/home/truckcontrol/test/jbussend_$DATESTR.dbg &
sleep 1
/home/truckcontrol/long_ctl/src/avcs/qnx/trk_wr -t 100 >/big/data/trk_wr$DATESTR.txt &
sleep 1
/home/truckcontrol/test/long_trk -v long_trk -f /home/truckcontrol/test/realtime.ini -o /big/data/realtime -r 1  &
