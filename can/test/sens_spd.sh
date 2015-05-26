#!/bin/sh
# Listen on can4 for raw sensor data, can1 for Jbus data
# Assumes janus_3can_1jbus.sh was used to start the drivers.
# vaa_sens_spd_rx saves about 5 minutes worth of data and exits.
# SPEED_SAMPLE specifies the number of sensor groups that are
# read before updating the speed that is being saved. The higher
# the number, the less communication between the threads and
# the less likelihood of needing to wait on a mutex. 

VAA_RX=/home/capath/can/client/path_can/qnx/vaa_sens_spd_rx_no_mutex
#VAA_RX=/home/capath/can/client/path_can/qnx/vaa_sens_spd_rx
SPEED_SAMPLE=5

$VAA_RX -p /dev/can4 -j /dev/can1 -s $SPEED_SAMPLE >/dev/shmem/sens_spd.out &
