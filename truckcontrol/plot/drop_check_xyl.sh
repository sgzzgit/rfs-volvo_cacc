#!/bin/sh
# Check Xiao-Yun's test.dat-type files for dropped data. This is done by 
# subtracting successive timestamps to see whether any differences
# are greater than 150 msec (should be 100). If so, the line number
# is returned.

rm blah
yold=0
lineno=1

cat $1 | awk '{print $1}'>blah
for x in `cat blah`
do 
	y=`echo $x | sed 's/\.//g'|sed 's/^000//g'|sed 's/^00//g'|sed 's/^0//g'`
	z=`echo $(($y-$yold))`
	yold=$y
	if [[ $z -gt 1500 ]] 
	then
		echo Time difference $z msec at line no $lineno
	fi
	lineno=$(($lineno+1))
done
