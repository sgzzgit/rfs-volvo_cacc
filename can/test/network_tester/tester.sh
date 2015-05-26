#####
# This script runs several tests between a pair of machines. For it to work,
# an appropriate slave script must be running on each machine. 
# 
# tester.sh [IP1] [IP2] [P1] [P2] [BAUD] [RATE] [N] [PRE] [IP]
#  [IPX] IP address of the machine to test
#  [PX] CAN port number on machine X
#  [BAUD] Transmission Rate
#  [RATE] number of messages to send each second
#  [N] number of messages to send in total
#  [PRE] files will be stored in directory [PRE][POST]
#        where [POST] is of the form 12U (1 -> 2)
#        U-unidirectional D-duplex
#
# Warnings: 
#  Creates a bunch of processes
#  No error checking
#  Not secure enough for public networks 
#
# Created by Sean Soleyman on Oct 5, 2009
#####

# Parameters
IP1=$1
IP2=$2
P1=$3
P2=$4
BAUD=$5
RATE=$6
N=$7
PRE=$8
IP=$9

echo "Testing connection between port:" $P1 on $IP1 and $P2 on $IP2
echo "Baud Rate:" $BAUD
echo "Message Rate:" $RATE
echo "Number of Messages:" $N
echo "File Names:" $PRE
echo "This Machine's IP:" $IP

# Ports (Note: CONTROL_PORT is also hardwired in the slave scripts)
CONTROL_PORT=3775
DATA1_PORT=3778
DATA2_PORT=3779

# This function redirects data from each TCP port to an appropriately-named file
# USAGE: redirect_all [POSTFIX]
# The files will be named ./[PREFIX][FROM][TO][NETWORK][POSTFIX]
function redirect_all {
    nc -l -p $DATA1_PORT > "${PRE}21${1}"&
    nc -l -p $DATA2_PORT > "${PRE}12${1}"&
}

# Start the machines at the correct baud rate
echo START $BAUD | nc -q 1 $IP1 $CONTROL_PORT
if [ $IP1 != $IP2 ] ; then
    echo START $BAUD | nc -q 1 $IP2 $CONTROL_PORT
fi
sleep 3

# Each set of tests will be performed in the following order: 
# 1. Machine 2->1
# 2. Machine 1->2

# Test 1: Unidirectional
redirect_all U
echo SEND $P2 $RATE $N 777 | nc -q 1 $IP2 $CONTROL_PORT
sleep 1
echo RECEIVE $P1 $IP $DATA1_PORT | nc -q 1 $IP1 $CONTROL_PORT
sleep 1
echo SEND $P1 $RATE $N 777 | nc -q 1 $IP1 $CONTROL_PORT
sleep 1
echo RECEIVE $P2 $IP $DATA2_PORT | nc -q 1 $IP2 $CONTROL_PORT
sleep 1

# Test 2: Bidirectional
# Not Yet Implemented
