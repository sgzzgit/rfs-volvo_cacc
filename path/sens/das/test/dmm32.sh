#!/bin/sh
	slay  -f -Q -s TERM veh_trk
	slay  -f -Q -s TERM db_slv
	/home/path/db/qnx/db_slv -Q -S `hostname` &
	sleep 1
	/home/truckcontrol/test/veh_trk -t 50 -v &
	slay  -f -Q -s TERM rddmm32
	slay  -f -Q -s TERM dmm32
	sleep 1
        qnx/dmm32 -v &
	sleep 1
	qnx/rddmm32 -o2 -v &
