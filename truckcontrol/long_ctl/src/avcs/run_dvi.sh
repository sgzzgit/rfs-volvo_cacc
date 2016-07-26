#!/bin/sh

/home/truckcontrol/long_ctl/src/avcs/slay_can.sh

IPADDR=`ifconfig | grep 172.16.0 | awk '{print $2}'`

echo $IPADDR | grep 75 
if [[ $? -eq 0 ]]
then
	ITRIADDR=172.16.1.75
	ITRIWIRELESS=172.16.2.75
	GALAXYWIRELESS=172.16.0.175
	TRUCK=Blue
fi

echo $IPADDR | grep 74 
if [[ $? -eq 0 ]]
then
	ITRIADDR=172.16.1.74
	ITRIWIRELESS=172.16.2.74
	GALAXYWIRELESS=172.16.0.174
	TRUCK=Gold
fi

echo $IPADDR | grep 76
if [[ $? -eq 0 ]]
then
	ITRIADDR=172.16.1.76
	ITRIWIRELESS=172.16.2.76
	GALAXYWIRELESS=172.16.0.176
	TRUCK=Silvr
fi

echo $IPADDR | grep 77
if [[ $? -eq 0 ]]
then
	ITRIADDR=172.16.1.77
	ITRIWIRELESS=172.16.2.77
	GALAXYWIRELESS=172.16.0.177
	TRUCK=Silvr
fi

DATESTR=`date +%m%d%Y_%H%M%S`
echo $DATESTR >/big/data/last_start_timestamp.txt
/home/truckcontrol/test/db_slv -Q -S `hostname` &
sleep 1
/home/truckcontrol/test/trk_cr -t 1000 2>long_input$DATESTR.dbg &
#sleep 1
#/home/can/drv_sja1000/qnx/can_man -n /dev/can1 -s 250 -i 10 -p 0xda000 -e 1 &
#sleep 1
#/home/can/drv_sja1000/qnx/can_man -n /dev/can2 -s 250 -i 7 -p 0xda200 -e 1 &
#sleep 1
#echo "Starting veh_snd from $IPADDR to $ITRIADDR"
#/home/truckcontrol/long_ctl/src/vehcomm/qnx/veh_snd -v -d -i 100 -A $IPADDR -a $ITRIADDR -u 15041 -t $TRUCK >veh_snd.dbg &
#sleep 1
#/home/truckcontrol/long_ctl/src/vehcomm/qnx/veh_rcv -v -A $IPADDR -a $ITRIADDR -u 15042 -t $TRUCK >veh_rcv.dbg &
#sleep 1
uname -s | grep Linux
if [[ $? -eq 0 ]]
then
	OBJDIR=lnx
fi
uname -s | grep QNX
if [[ $? -eq 0 ]]
then
	OBJDIR=qnx
fi
OBJDIR=qnx
echo "Turning on startup screen"
../vehcomm/$OBJDIR/dvi_snd -c -C 20 -r 10007 -R 10005 -a $GALAXYWIRELESS -A $IPADDR -E "3 0 0 1 0 0 0 0 0 0" -P "0 1 0 0 0 1 0 0 1 1 1 0 0 0 "
sleep 1
echo "Turning on startup screen"
../vehcomm/$OBJDIR/dvi_snd -c -C 20 -r 10007 -R 10005 -a $GALAXYWIRELESS -A $IPADDR -E "3 0 0 1 0 0 0 0 0 0" -P "2 1 0 0 0 1 0 0 1 1 1 0 0 0 "
sleep 1
echo "Turning on platoon found screen"
../vehcomm/$OBJDIR/dvi_snd -c -C 20 -r 10007 -R 10005 -a $GALAXYWIRELESS -A $IPADDR -E "3 0 0 1 0 0 0 0 0 0" -P "2 1 0 1 0 1 0 0 1 1 1 0 0 0 "
sleep 2
echo "Turning on startup screen"
../vehcomm/$OBJDIR/dvi_snd -c -C 20 -r 10007 -R 10005 -a $GALAXYWIRELESS -A $IPADDR -E "3 0 0 1 0 0 0 0 0 0" -P "0 1 0 0 0 1 0 0 1 1 1 0 0 0 "
slay trk_cr
slay db_slv
