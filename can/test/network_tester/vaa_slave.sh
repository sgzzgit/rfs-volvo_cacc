#####
# This script listens on port 3775, and responds to the following TCP messages:
#  RECEIVE [CAN PORT] [TCP IP] [TCP PORT]
#   retransmits data from a CAN port to a TCP port
#  SEND [CAN PORT] [RATE] [N] [IDENTIFIER]
#   sends messages on a CAN port
#  START [BAUD]
#   starts the CAN drivers at the specified speed
#  TERMINATE
#   stops this program
#
# Example of Usage: echo TERMINATE | nc -q 192.168.1.101 3775
#
# Created by Sean Soleyman Oct 4 2009
#####

#TEST_DIR contains can_rx and can_tx
TEST_DIR=/home/capath/can/client/path_can/qnx
#SCRIPT_DIR contains can_start.sh and can_stop.sh
SCRIPT_DIR=/home/sean/network_tester/stub

# Initialize Command Handler
CONTROL_PORT=3775
COMMAND=`nc -l -p $CONTROL_PORT`
for word in $COMMAND; do
    ARGV[${ARGC}]=$word
    ARGC=$ARGC+1
done
	echo test
while [ ${ARGV[0]} != "TERMINATE" ]; do
    ARGC=0;
    echo "COMMAND RECEIVED: " ${ARGV[*]}

    # START
    if [ ${ARGV[0]} = "START" ]; then
	$SCRIPT_DIR/can_slay.sh
	sleep 1
	$SCRIPT_DIR/can_start.sh ${ARGV[1]}
	sleep 1
    fi

    # SEND
    if [ ${ARGV[0]} = "SEND" ]; then
	$TEST_DIR/can_tx -p ${ARGV[1]} -e 1 -t ${ARGV[2]} -n ${ARGV[3]} -i ${ARGV[4]}&
	echo sending ${ARGV[1]}
    fi

    # RECEIVE
    if [ ${ARGV[0]} = "RECEIVE" ]; then
	$TEST_DIR/can_rx -p ${ARGV[1]} | nc -c ${ARGV[2]} ${ARGV[3]}&
	echo retransmitting to port ${ARGV[3]} on ${ARGV[2]}
    fi

    # STOP
    if [ ${ARGV[0]} = "STOP" ]; then
	$SCRIPT_DIR/can_slay
    fi

    # Fetch the Next Command
    COMMAND=`nc -l -p $CONTROL_PORT`
    for word in $COMMAND; do
	ARGV[${ARGC}]=$word
	ARGC=$ARGC+1
    done
done
