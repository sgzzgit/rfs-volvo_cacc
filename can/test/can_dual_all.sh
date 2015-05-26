# Use this to start the DACHS SJA (steinhoff) driver can_dual for the Janus-MM 
# along with a send and receive process for each port.
#
# Data from the can_rx process is piped to nc to be received on 
# a REMOTE system; nc receiving processes must be started on the
# REMOTE system first. 
#
# Setting of shell variables at beginning of script can be easily
# changed to initialize them to arguments, as is done for the send
# interval.

CLIENT_DIR=/home/capath/can/client/steinhoff/qnx
SELF=101
INTERVAL=$1
PORT1=3777
PORT2=3779
REMOTE=192.168.1.104

slay  -f -Q can_dual
sleep 1
(cd /DACHS-QNX6.3/CAN-SJA1000-104-1.0.1-JAND/src; ./can_dual -B 0x01 -b 0x1c -C 0x01 -c 0x1c &)
sleep 1
$CLIENT_DIR/can_tx -p 1 -i 1$SELF -e 1 -t $INTERVAL &
$CLIENT_DIR/can_tx -p 2 -i 2$SELF -e 1 -t $INTERVAL &
sleep 1
$CLIENT_DIR/can_rx -p 1  | nc -c $REMOTE $PORT1 &
$CLIENT_DIR/can_rx -p 2  | nc -c $REMOTE $PORT2 &
