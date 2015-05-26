#!/bin/sh
# Check trk_wr.dat-type files for dropped data. This is done by 
# converting the timestamp in the first column to seconds-since-midnight
# with fixtime.sh, and then subtracting successive times to see whether
# any differences are greater than 75 msec (should be 50). If so, the 
# line number is returned.

rm blah
yold=0
lineno=1

if [[ -e test.dat ]] 
then
	mv test.dat zzxxzz
fi

cp $1 test.dat
/home/path/plot/fixtime.sh test.dat

cat test.plt | awk '{print $1}'>blah
for x in `cat blah`
do 
	y=`echo $x | sed 's/\.//g'|sed 's/^000//g'|sed 's/^00//g'|sed 's/^0//g'`
	z=`echo $(($y-$yold))`
	yold=$y
	if [[ $z -gt 75 ]] 
	then
		echo Time difference $z msec at line no $lineno
	fi
	lineno=$(($lineno+1))
done

if [[ -e zzxxzz ]] 
then
	mv zzxxzz test.dat
fi
