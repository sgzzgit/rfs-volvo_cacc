#!/bin/sh
#
# Script to run device drivers and longitudinal control code

TEST_DIR=/home/truckcontrol/test
CAN_CLIENT_DIR=/home/can/jbus/src/qnx
CAN_DRIVER_DIR=/home/can/drv_sja1000/qnx

	slay  -f -Q -s TERM dvi_snd 
	slay  -f -Q -s TERM dvi_rcv
	slay  -f -Q -s TERM trk_wr
	slay  -f -Q -s TERM rdj1939
	slay  -f -Q -s TERM veh_snd
	slay  -f -Q -s TERM veh_rcv
	slay  -f -Q -s TERM gpsdb
	slay  -f -Q -s TERM jbussendGPS
	slay  -f -Q -s TERM jbussend
	slay  -f -Q -s TERM can_man
	slay  -f -Q -s TERM trk_cr
	slay  -f -Q -s TERM db_slv

	DATESTR=`date +%m%d%Y_%H%M%S`
	echo $DATESTR >/big/data/last_start_timestamp.txt

	IPADDR=`ifconfig | grep 172.16.0 | awk '{print $2}'`

	echo $IPADDR | grep 74 
	if [[ $? -eq 0 ]]
	then
	        ITRIADDR=172.16.1.74
	        GALAXYWIRELESS=172.16.0.174
	        TRUCK=Gold
	fi

	echo $IPADDR | grep 75 
	if [[ $? -eq 0 ]]
	then
	        ITRIADDR=172.16.1.75
	        GALAXYWIRELESS=172.16.0.175
	        TRUCK=Blue
	fi

	echo $IPADDR | grep 76
	if [[ $? -eq 0 ]]
	then
	        ITRIADDR=172.16.1.76
	        GALAXYWIRELESS=172.16.0.176
	        TRUCK=Silvr
	fi

	echo $IPADDR | grep 77 
	if [[ $? -eq 0 ]]
	then
	        ITRIADDR=172.16.1.77
	        GALAXYWIRELESS=172.16.0.177
	        TRUCK=Blue
	fi

	$TEST_DIR/db_slv -Q -S `hostname` &
	sleep 1
	$TEST_DIR/trk_cr -t 1000 2>long_input_$DATESTR.dbg &
	sleep 1
	nice --2 $CAN_DRIVER_DIR/can_man -n /dev/can1 -s 250 -i 10 -p 0xda000 -e 1 >/big/data/can1$DATESTR.log &
	sleep 1
	nice --2 $CAN_DRIVER_DIR/can_man -n /dev/can2 -s 500 -i 7 -p 0xda200 -e 1 >/big/data/can2$DATESTR.log &
	sleep 1
	nice --1 $CAN_CLIENT_DIR/rdj1939 -c -f /dev/can1 >/big/data/rdj1939_can1_$DATESTR.txt &
	sleep 1
	nice --1 $CAN_CLIENT_DIR/rdj1939 -c -f /dev/can2 >/big/data/rdj1939_can2_$DATESTR.txt &
	sleep 1
	# To start jbussend with debugging ON, add -d flag
	nice --2 $TEST_DIR/jbussend -d -c -e /dev/can1 -b /dev/can1 >/big/data/jbussend_$DATESTR.dbg &
	sleep 1
# To start jbussendGPS with debugging ON, add -d flag
	GPSDBNUM=202
	$TEST_DIR/jbussendGPS -c -g /dev/can2 -n $GPSDBNUM >/big/data/jbussendGPS_$DATESTR.dbg &
	sleep 1
	$TEST_DIR/gpsdb -n $GPSDBNUM -d1 </dev/ser1 >/big/data/gpsdb_$DATESTR.dbg &
	sleep 1
	/home/truckcontrol/test/trk_wr -t 100 1>/big/data/trk_wr_$DATESTR.txt 2>/big/data/trk_wr.err &
	sleep 1
	$TEST_DIR/veh_snd -v -i 100 -A $IPADDR -a $ITRIADDR -u 15041 -t $TRUCK >/big/data/veh_snd_$DATESTR.dbg &
	sleep 1
	$TEST_DIR/veh_rcv -f $CONFIG_FILE -v -A $IPADDR -a $ITRIADDR -u 15042 -t $TRUCK >/big/data/veh_rcv_$DATESTR.dbg &
	sleep 1
	$TEST_DIR/dvi_rcv -u 8003 -a $GALAXYWIRELESS -A $IPADDR >/big/data/dvi_rcv_$DATESTR.dbg &
	sleep 1
	$TEST_DIR/dvi_snd -r 10007 -R 10005 -a $GALAXYWIRELESS -A $IPADDR >/big/data/dvi_snd_$DATESTR.dbg &

	nice --2 $TEST_DIR/long_trk -v long_trk -f $TEST_DIR/realtime.ini -o test_`cat /big/data/last_start_timestamp.txt` -r 1 >/big/data/long_trk.log 2>/big/data/long_trk.err &
