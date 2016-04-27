#!/bin/sh
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

/home/truckcontrol/long_ctl/src/vehcomm/qnx/veh_rcv -v -A $IPADDR -a $ITRIADDR -u 15042 -t $TRUCK
