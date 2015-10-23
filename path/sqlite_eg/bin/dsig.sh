#!/bin/sh
# run ptrel to create table in other_gps.db containing coordinates
# of points relative to RFS traffic signal (GPS taken from Joel's
# database), with coordinate Y axis north.

ptrel -v -h 0.0 -i "rel_gps.db" -o "rel_gps.db" -x 37.915571 -y -122.334853 
