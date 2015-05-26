#!/bin/sh
# Script to take the output of can_rx5 when receiving from a J1939
# bus, filter out the CCVS (Cruise Control Vehicle Speed) messages
# and convert the speed, adding three more columns, with the speed
# (meters/second) in the rightmost column.
#
# Usage: filter_speed.sh can_rx5.out
#
# Get can_rx5.out by running
# cd /home/capath/can/client/path_can/qnx
# can_rx5 -p /dev/can1 >/dev/shmem/can_rx5.out
#
# 
# NOTE: I might have the high byte and the low byte of the speed 
# confused, if so, just reverse $13 and $12 in the last part of
# the expression at the end below.
#
awk '{print $0, and(rshift($2,16),255), and(rshift($2,8),255)}' $1 | grep '254 241' | awk '{print $0, (($4<0)?($4+256):$4), (($5<0)?($5+256):$5)}' | awk '{print $0, ($13 +$12/256.0)*(1000.0/3600.0)}'

