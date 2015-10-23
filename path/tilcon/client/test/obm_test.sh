#!/bin/sh
# This "script" is just a reminder of what processes to run
# since chg_vals, which requires interactive input and
# cannot be started in the background, must be started before obm_test

#start the database
/home/path/db/db_slv &

sleep 1
# start chg_vals before obm_test, to create DB_OBM_RADAR and DB_OBM_METER
echo 'start obm_test in another window'
./obm_chg_vals 

sleep 1
# start obm_test
#./obm_test

