#!/bin/sh
# Takes a file begining with two hr:min:s.milli timestamps and produces
# an output file where the timestamps have been converted to a decimal
# number of seconds since midnight 
#
# Assuming initial file is $base.dat, output file will be $base.plt
# Script takes one argument, the name of the initial file
base=`echo $1 | sed 's/.dat//'`
cat $1 | sed 's/:/ /g' | awk -f /home/path/plot/fixradar.awk >$base.plt 
