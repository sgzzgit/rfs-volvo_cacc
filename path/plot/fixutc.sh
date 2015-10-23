#!/bin/sh
# Takes a file in xfile format with a utc timestamp in column 37 
# and produces an output file an output file where the utc timestamp
# has been converted to a decimal number of seconds since midnight 
# (Greenwich time)
#
# Assuming initial file is $base.plt, output file will be $base.plt2
# Script takes one argument, the name of the initial file
# 
# Meant to be used after fixtime.sh; at some point we should combine
# the awk scripts and make a fixxfile.sh
base=`echo $1 | sed 's/.plt//'`
cat $1 | sed 's/:/ /g' | awk -f fixutc.awk >$base.plt2 
