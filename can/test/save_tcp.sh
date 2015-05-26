#!/bin/sh
# Saves data sent over TCP port $1 to file $2
/usr/local/bin/nc -l -p $1 >$2 
