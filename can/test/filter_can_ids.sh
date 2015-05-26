#!/bin/sh
# Script to take the output of can_rx5 when receiving from a J1939
# bus, and show the PDU FORMAT and PDU SPECIFIC components of
# the ID, which can be used to identify the message types being sent.
#
# Usage: filter_can_ids.sh can_rx5.out
#
# Get can_rx5.out by running
# cd /home/capath/can/client/path_can/qnx
# can_rx5 -p /dev/can1 >/dev/shmem/can_rx5.out
#
#
awk '{print $0, and(rshift($2,16),255), and(rshift($2,8),255)}' $1 

