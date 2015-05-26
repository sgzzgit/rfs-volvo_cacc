#!/bin/sh
# Stop processes in sens_spd.sh 

slay -f -Q vaa_sens_spd_rx
slay -f -Q vaa_sens_spd_rx_no_mutex
