#!/bin/sh

slay  -f -Q -s TERM long_trk
slay  -f -Q -s TERM dvi_rcv
slay  -f -Q -s TERM dvi_snd
slay  -f -Q -s TERM veh_snd
slay  -f -Q -s TERM veh_rcv
slay  -f -Q -s TERM trk_wr
slay  -f -Q -s TERM rdj1939
slay  -f -Q -s TERM gpsdb
slay  -f -Q -s TERM jbussendGPS
slay  -f -Q -s TERM jbussend
slay  -f -Q -s TERM can_man
slay  -f -Q -s TERM trk_cr
slay  -f -Q -s TERM db_slv
if [ -e /big/data/last_start_timestamp.txt ]
then
	if [ -e /big/data/test.dat ]
	then
		mv /big/data/test.dat /big/data/test`cat /big/data/last_start_timestamp.txt`.txt
	fi
	rm /big/data/last_start_timestamp.txt
fi
