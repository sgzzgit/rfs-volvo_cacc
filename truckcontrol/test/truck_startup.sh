#!/bin/sh
#
# Script to run, from startup, device drivers
# for BAA project 2008-10 
# Usage, e.g., truck_startup.sh -V Blue

while getopts "V:" opt; do
case $opt in
    V) TRUCK=$OPTARG ;;
esac
done
if [[ $OPTIND -lt 3 ]] ; then
        echo 'No vehicle color specified (must be -V <Blue, Gold, or Silvr)'
        exit
fi

if [[ $TRUCK != "Blue" && $TRUCK != "Gold" && $TRUCK != "Silvr" ]] ; then
	echo $TRUCK is neither Blue, Gold, nor Silvr
	exit
fi

echo optind $OPTIND

CONFIG_FILE='/home/truckcontrol/test/realtime.ini' ;
TEST_DIR='/home/truckcontrol/test' ;
CAN_CLIENT_DIR='/home/can/client/path_can/qnx' ;
CAN_DRIVER_DIR='/home/can/drv_i82527/qnx' ;
DATAFILE='/big/data/trk_wr.dat' ;

if [[ $TRUCK == "Blue" ]] ; then
	DSRC_IP='10.0.1.34' ;
fi
if [[ $TRUCK == "Gold" ]] ; then
	DSRC_IP='10.0.1.46' ;
fi
if [[ $TRUCK == "Silvr" ]] ; then
	DSRC_IP='10.0.1.7' ;
fi
echo 'Vehicle color' $TRUCK
echo 'Run data server and create variables'
        slay  -f -Q -s TERM trk_cr
        slay  -f -Q -s TERM db_slv
        $TEST_DIR/db_slv -Q -S `hostname` &
        sleep 1
        $TEST_DIR/trk_cr -t 1000 2>long_input.dbg &

echo 'Kill can_man and jbussend'
        slay  -f -Q -s TERM jbussend
        slay  -f -Q -s TERM can_man
        sleep 1

echo 'Start can_man and jbussend'
        $CAN_DRIVER_DIR/can_man -n /dev/can1 -s 250 -i 10 -p 0xd8000 -e 1 >/home/truckcontrol/test/can1.log &
        sleep 1
        $CAN_DRIVER_DIR/can_man -n /dev/can2 -s 250 -i 7 -p 0xd8100 -e 1 >/home/truckcontrol/test/can2.log &
        sleep 1
        # To start jbussend with debugging ON, add -d flag
        $TEST_DIR/jbussend -c -e /dev/can1 -b /dev/can2 >/home/truckcontrol/test/jbussend.dbg &
        sleep 1
echo 'Run device drivers & v2v communication, and start logging'
        slay  -f -Q -s TERM trk_io
        slay  -f -Q -s TERM dmm32
        slay  -f -Q -s TERM evt300
        slay  -f -Q -s TERM rdj1939
        slay  -f -Q -s TERM veh_snd
        slay  -f -Q -s TERM veh_rcv
        slay  -f -Q -s TERM gpsrcv

if [[ $TRUCK == "Silvr" ]] ; then
	slay -f -Q -s TERM mdl;
	$TEST_DIR/mdl -o3 </dev/ser2 >mdl.log &
fi
if [[ $TRUCK == "Gold" ]] ; then
	slay -f -Q -s TERM rdlidar;
	$TEST_DIR/rdlidar -o 2 < /dev/ser4 >densolidar.log &
fi
        $TEST_DIR/gpsrcv -n 202 -u 5050  >gpsrcv.dbg &
        $TEST_DIR/veh_rcv -f $CONFIG_FILE -u 5051  -t $TRUCK >veh_rcv.dbg &
        $TEST_DIR/veh_snd -i 50 -a $DSRC_IP -u 5052  -t $TRUCK >veh_snd.dbg &
        sleep 1
        $CAN_CLIENT_DIR/rdj1939 -c -f /dev/can1 >j1939_eng.dbg &
        sleep 2
        $CAN_CLIENT_DIR/rdj1939 -c -f /dev/can2 >j1939_brake.dbg &
        sleep 2
        $TEST_DIR/evt300 -s "/dev/ser1" -r a -o 2 >evt300a.log &
        sleep 1
       $TEST_DIR/dmm32 >dmm32.log &
        sleep 1
        $TEST_DIR/trk_io -o2 >trk_io.log 2>trk_io.err &
