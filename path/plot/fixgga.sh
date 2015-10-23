#!/bin/sh
# Takes a file containing NEMA GPS lines, selects GGA, replaces , with space 
# and fixes utc to a decimal number of seconds since midnight 
#
# Assuming initial file is $base.*, output file will be $base.plt
# Script takes one argument, the name of the initial file
base=`echo $1 | sed 's/\.*//'`
cat $1 | grep GGA | sed 's/,/ /g' | awk -f fixgga.awk >$base.plt 
