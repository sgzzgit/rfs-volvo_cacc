# Use this to start can_man with the ECAN i82527 board
CAN_DRV_DIR=/home/can/drv_i82527/qnx
$CAN_DRV_DIR/can_man -n /dev/can1 -s 1000 -i 10 -p 0xd8000 -e 1 > can_man1.log  &
sleep 1
$CAN_DRV_DIR/can_man -n /dev/can2 -s 1000 -i 7 -p 0xd8100 -e 1 > can_man2.log &
sleep 1
