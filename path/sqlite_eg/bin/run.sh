#!/bin/sh
# gets rid of old sqlite3 path_gps.db
# creates empty databases from gps.dump
# runs gpsinsert in background
# ptsnd queries my_gps.db pttbl filled by gpsinsert and sends out message 
# ptrcv gets message from neighbors (and self) and fills in other_gps.db
# process to do coordinate transform and create neighbortbl in other_gps.db
# under development
cd /tmp/fs
rm -f my_gps.db
rm -f other_gps.db
sqlite3 my_gps.db </home/path/sqlite_eg/bin/gps.dump
sqlite3 other_gps.db </home/path/sqlite_eg/bin/gps.dump
sleep 1
#cat </dev/ttyUSB0 | gpsinsert -t >gpsinsert.out &
/home/path/sens/gps/examples/tcp_client -q r | /home/path/sqlite_eg/bin/gpsinsert -t &>gpsinsert.out &
sleep 1
/home/path/sqlite_eg/bin/ptrcv  &>ptrcv.out &
sleep 1
/home/path/sqlite_eg/bin/ptsnd &>ptsnd.out &
