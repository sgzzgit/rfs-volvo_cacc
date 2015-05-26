#!/bin/sh
#
# Script to stop device drivers and longitudinal control code
# for BAA project 2008-10 

echo 'Stop longitudinal control'
        slay  -f -Q -s TERM long_trk
        sleep 1
echo 'Stop device drivers, communication and logging'
        slay  -f -Q -s TERM trk_wr
        slay  -f -Q -s TERM trk_io
        slay  -f -Q -s TERM dmm32
        slay  -f -Q -s TERM rdlidar
        slay  -f -Q -s TERM mdl
        slay  -f -Q -s TERM evt300
        slay  -f -Q -s TERM rdj1939
#       slay  -f -Q -s TERM rdj1587
        slay  -f -Q -s TERM veh_snd
        slay  -f -Q -s TERM veh_rcv
        slay  -f -Q -s TERM gpsrcv
        sleep 1
echo 'Kill can_man and jbussend'
        slay  -f -Q -s TERM jbussend
        slay  -f -Q -s TERM can_man
        sleep 1
echo 'Stop data server'
        slay  -f -Q -s TERM trk_cr
        sleep 1
        slay  -f -Q -s TERM db_slv
