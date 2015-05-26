#!/bin/sh
#
# Script to run device drivers and longitudinal control code
# for BAA project 2008-10 

CONFIG_FILE='/home/truckcontrol/test/realtime.ini' ;
PLATOON_SIZE='1' ;
STEERING='trk_lat' ;
LONGITUDINAL='long_trk' ;
LONG_DIR='/home/truckcontrol/test/' ;
TEST_DIR='/home/truckcontrol/test' ;
CAN_CLIENT_DIR='/home/can/client/path_can/qnx' ;
CAN_DRIVER_DIR='/home/can/drv_i82527/qnx' ;
LONG_CFG_FILE='realtime' ;
LONG_PLOT='/home/truckcontrol/plot/trkplot' ;
COMMAND_TEST='1' ;
LAPTOP_IP='128.32.235.133' ;
DSRC_IP='10.0.1.46' ;
DATAFILE='/big/data/trk_wr.dat' ;
USER='jspring' ;
PLOT_DIR='/home/jspring/plot' ;

	slay  -f -Q -s TERM jbussend
	slay  -f -Q -s TERM can_man
	slay  -f -Q -s TERM trk_cr
	slay  -f -Q -s TERM db_slv
	$TEST_DIR/db_slv -Q -S `hostname` &
	sleep 1
	$TEST_DIR/trk_cr -t 1000 2>long_input.dbg &
	sleep 1
	$CAN_DRIVER_DIR/can_man -n /dev/can1 -s 250 -i 10 -p 0xd8000 -e 1 >/home/truckcontrol/test/can1.log &
	sleep 1
	$CAN_DRIVER_DIR/can_man -n /dev/can2 -s 250 -i 7 -p 0xd8100 -e 1 >/home/truckcontrol/test/can2.log &
	sleep 1
	# To start jbussend with debugging ON, add -d flag
	$TEST_DIR/jbussend -c -e /dev/can1 -b /dev/can2 >/home/truckcontrol/test/jbussend.dbg &
	sleep 1
	$TEST_DIR/long_out -v 950 &
	sleep 4
	slay  -f -Q -s TERM long_out
	$TEST_DIR/long_out -b -1.0 &
	sleep 4

	slay  -f -Q -s TERM long_out
	slay  -f -Q -s TERM jbussend
	slay  -f -Q -s TERM can_man
	slay  -f -Q -s TERM trk_cr
	slay  -f -Q -s TERM db_slv
