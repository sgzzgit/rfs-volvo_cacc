#!/bin/sh
# sndpdufile sends J1939 messages from a file until terminated.
# can_man driver for DEVFILE must have -b 250 (250K bitspeed)
# and -e 1 (extended header) to match J1939 standard
SNDPDU=/home/capath/can/jbus/src/qnx/sndpdufile
DEVFILE=/dev/can1
INFILE=/home/capath/can/jbus/data/truck/source1.dat
$SNDPDU -c -i $INFILE -f $DEVFILE -t 1  &
