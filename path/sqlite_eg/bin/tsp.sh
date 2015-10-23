#!/bin/sh
# gets rid of old sqlite3 path_gps.db
# creates empty databases from gps.dump
# runs gpsinsert in background
# ptsnd queries my_gps.db pttbl filled by gpsinsert and sends out message 
# ptrcv gets message from neighbors (and self) and fills in other_gps.db
# process to do coordinate transform and create neighbortbl in other_gps.db
# under development
#cp /home/gems/data/jun16/my_gps.db trace_gps.db
sleep 1
/home/path/db/db_slv &
sleep 1
# ptreplay is needed only for testing, can receive from ptsnd ordinarily.
# by default ptsnd goes to port 7015; when using ptreplay in a room
# where other systems may be running ptsnd, use a different port
# for tsprcv and ptreplay.
#/home/path/sqlite_eg/bin/ptreplay -v -u 7017 -a 127.0.0.1 -d trace_gps.db >ptreplay.out &
sleep 1
/home/path/sens/gps/examples/tsprcv -c -v -u 7015 >tsprcv.out &
