#Use this to start can_man with the Janus-MM Phillips SJA 1000 board
CAN_DIR=/home/capath/can/drv_sja1000/qnx
LOG_DIR=/dev/shmem
BIT_SPEED=1000
IRQ1=5
IRQ2=7
EXTENDED=1
slay  -f -Q -s TERM can_man
$CAN_DIR/can_man -n /dev/can1 -s $BIT_SPEED -i $IRQ1 -p 0xd0000 -e $EXTENDED >$LOG_DIR/can_man1.log  &
sleep 1
$CAN_DIR/can_man -n /dev/can2 -s $BIT_SPEED -i $IRQ2 -p 0xd0200 -e $EXTENDED >$LOG_DIR/can_man2.log &
sleep 1
