#####
# Simple script to test vaa_slave.sh and bidirectional traffic.
# Running on the Linux laptop
#
# Parameters are taken from input files, to make them easier
# to change with an editor.
#
# These files can be created by a script that calls this one.
# 
# Created by Sue Dickey, Oct 15, 2009
#####

# Ports (Note: CONTROL_PORT is also hardwired in the slave scripts)
CONTROL_PORT=3775

# v1v2 will hold CAN data from VAA2 to VAA1 
# v2v1 will hold CAN data from VAA1 to VAA2 
# currently these are connected on the 2nd CAN prot
nc -l -p 3776 > v1v2_3776.dat &
nc -l -p 3777 > v2v1_3777.dat &
sleep 1

# restart the driver every time, because it has been hanging
# periodically on VAA1
echo START $1 | nc -q 0 192.168.1.101 $CONTROL_PORT 
echo START $1 | nc -q 0 192.168.1.102 $CONTROL_PORT 

# Make sure above nc receivers are started before sending can_rx output over nc
sleep 2
nc -q 0 192.168.1.101 $CONTROL_PORT <receive2_101.in
nc -q 0 192.168.1.102 $CONTROL_PORT <receive2_102.in

# Don't start CAN send too soon after receivers are started
sleep 1
nc -q 0 192.168.1.101 $CONTROL_PORT <send2_101.in
nc -q 0 192.168.1.102 $CONTROL_PORT <send2_102.in
 
