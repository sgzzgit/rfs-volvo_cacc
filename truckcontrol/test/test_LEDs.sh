#!/bin/sh
#
# Script to run, from startup, device drivers
# for BAA project 2008-10 

TEST_DIR='/home/truckcontrol/test' ;

echo "Starting program to test all LEDs"
echo
echo "Let's kill everything first"
echo
/home/truckcontrol/test/truck_stop.sh
echo
echo "Start up the data server and dmm32 I/O"

        $TEST_DIR/db_slv -Q -S `hostname` &
        sleep 1
        $TEST_DIR/trk_cr -t 1000 2>long_input.dbg &
        sleep 1
        $TEST_DIR/dmm32 >dmm32.log &
        sleep 1
        $TEST_DIR/trk_io -o2 >trk_io.log &
echo
echo "OK, now let's test the LEDs"
echo
        sleep 1
        $TEST_DIR/set_fault -h
echo "Set red only"
        sleep 5
        $TEST_DIR/set_fault -l
echo "Set green and amber"
        sleep 5
        $TEST_DIR/set_fault -m
echo "Set green and amber flash"
        sleep 5
        $TEST_DIR/set_fault -a
echo "Set green only"
        sleep 5
        $TEST_DIR/set_fault -c
echo "Set amber only"
        sleep 5
        $TEST_DIR/set_fault -r
echo "Set blue and green"
        sleep 5
        $TEST_DIR/set_fault -z
echo "Turn all LEDs off"
