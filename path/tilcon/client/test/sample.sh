#!/bin/sh
# This "script" is just a reminder of what processes to run
# since chg_vals, which requires interactive input and
# cannot be started in the background, must be started before sample

#start the database
/home/path/db/lnx/db_slv &

sleep 1
# start chg_vals before sample, to create DB_OBM_RADAR and DB_OBM_METER
echo 'start sample in another window'
../src/lnx/chg_vals 

sleep 1
# start sample
#../src/lnx/sample

