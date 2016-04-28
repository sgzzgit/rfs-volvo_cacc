#!/bin/sh
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

/home/truckcontrol/long_ctl/src/vehcomm/qnx/veh_rcv -v -A $IPADDR -a $ITRIADDR -u 15042 -t $TRUCK
