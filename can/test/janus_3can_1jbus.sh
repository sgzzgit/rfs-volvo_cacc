#Use this to start can_man with the Janus-MM Phillips SJA 1000 board
CAN_DIR=/home/capath/can/drv_sja1000/qnx
BIT_SPEED=1000
JBUS_BIT_SPEED=250
EXTENDED=0
slay  -f -Q -s TERM can_man
$CAN_DIR/can_man -n /dev/can1 -s $BIT_SPEED  -i 5 -p 0xd0000 -e $EXTENDED >/dev/shmem/can_man1.log  &
sleep 1
$CAN_DIR/can_man -n /dev/can2 -s $BIT_SPEED -i 7  -p 0xd0200 -e $EXTENDED  >/dev/shmem/can_man2.log &
sleep 1
$CAN_DIR/can_man -n /dev/can3 -s $BIT_SPEED -i 15 -p 0xd1000 -e $EXTENDED >/dev/shmem/can_man3.log  &
sleep 1
# J1939 bus always has extended header
$CAN_DIR/can_man -n /dev/can4 -s $JBUS_BIT_SPEED -i 9  -p 0xd1200 -e 1 >/dev/shmem/can_man4.log &
sleep 1
