#!/bin/sh
#
# Script to run, from startup, device drivers
# for BAA project 2008-10 

TEST_DIR='/home/truckcontrol/test' ;
CAN_CLIENT_DIR='/home/can/client/path_can/qnx' ;
CAN_DRIVER_DIR='/home/can/drv_i82527/qnx' ;
echo 'Kill can_man, jbussend, and can_rx'
        slay  -f -Q -s TERM can_rx
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
echo
echo
echo 'Testing can1 with can_rx'
echo
echo
        sleep 2
	$CAN_CLIENT_DIR/can_rx -p /dev/can1 &
        sleep 5
        slay  -f -Q -s TERM can_rx
	if [[ `ps -aef | grep -v grep | grep can_man | grep 'dev/can1'` == '' ]]
	then
		echo
		echo can_man for can1 not running!!!
		echo
	fi
echo
echo
echo 'Testing can1 with rdj1939'
echo
echo
        sleep 2
	$CAN_CLIENT_DIR/rdj1939 -v -c -f /dev/can1 &
        sleep 5
        slay  -f -Q -s TERM rdj1939
	if [[ `ps -aef | grep -v grep | grep can_man | grep 'dev/can1'` == '' ]]
	then
		echo
		echo can_man for can1 not running!!!
		echo
	fi
echo
echo
echo 'Testing can2 with can_rx'
echo
echo
        sleep 2
	$CAN_CLIENT_DIR/can_rx -p /dev/can2 &
        sleep 5
        slay  -f -Q -s TERM can_rx
	if [[ `ps -aef | grep -v grep | grep can_man | grep 'dev/can2'` == '' ]]
	then
		echo
		echo can_man for can2 not running!!!
		echo
	fi
echo
echo
echo 'Testing can2 with rdj1939'
echo
echo
        sleep 2
	$CAN_CLIENT_DIR/rdj1939 -v -c -f /dev/can2 &
        sleep 10
        slay  -f -Q -s TERM rdj1939
        slay  -f -Q -s TERM jbussend
        slay  -f -Q -s TERM can_man
	if [[ `ps -aef | grep -v grep | grep can_man | grep 'dev/can2'` == '' ]]
	then
		echo
		echo can_man for can2 not running!!!
		echo
	fi
