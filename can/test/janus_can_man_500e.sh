#Use this to start can_man with the Janus-MM Phillips SJA 1000 board
CAN_DIR=/home/capath/can/drv_sja1000/qnx
slay  -f -Q -s TERM can_man
$CAN_DIR/can_man -n /dev/can1 -s 500 -i 7 -p 0xd0000 -e 1 >/dev/shmem/can_man1.log  &
sleep 1
$CAN_DIR/can_man -n /dev/can2 -s 500 -i 7 -p 0xd0200 -e 1 >/dev/shmem/can_man2.log &
sleep 1
