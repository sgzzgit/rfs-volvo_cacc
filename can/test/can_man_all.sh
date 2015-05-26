# Use this to start can_man with the Janus-MM Phillips SJA 1000 board,
# along with a send and receive process for each port.
#
# path_can drivers currently have the bug that if there is a tx process
# on a channel, it must be started when no rx processes are active,
# or the rx process will hang after the tx process opens the device.
#
# Data from the can_rx process is saved to a file on /dev/shmem 
# If this runs for too long it will fill the memory, so limit
# the sends from the remote system.
#
CAN_DIR=/home/capath/can/drv_sja1000/qnx
CLIENT_DIR=/home/capath/can/client/path_can/qnx
SELF=101
INTERVAL=$1
REMOTE=192.168.1.104

slay  -f -Q -s TERM can_man
sleep 1
$CAN_DIR/can_man -n /dev/can1 -s 250 -i 11 -p 0xd0000 -e 1 >/dev/shmem/can_man1.log &
sleep 1
$CAN_DIR/can_man -n /dev/can2 -s 250 -i 11 -p 0xd0200 -e 1 >/dev/shmem/can_man2.log &
sleep 1
$CLIENT_DIR/can_tx -p /dev/can1 -i 1$SELF -e 1 -t $INTERVAL &
$CLIENT_DIR/can_tx -p /dev/can2 -i 2$SELF -e 1 -t $INTERVAL &
sleep 1
$CLIENT_DIR/can_rx -p /dev/can1  >/dev/shmem/rx1.out &
$CLIENT_DIR/can_rx -p /dev/can2  >/dev/shmem/rx2.out &   
