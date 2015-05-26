#!/bin/sh
# Creates input files and calls simple.sh multiple times
FILE1=v1v2_3776.dat
FILE2=v2v1_3777.dat

declare -i RUN_LENGTH
RUN_LENGTH=10000
declare -i COUNT

mkdir -p v1v2
mkdir -p v2v1

# Outer loop is over bit speed, inner loop is over millisecond send interval
for b in 250 500
do
	for t in 100 050 020
	do
		echo $b $t
		echo SEND 2 0.$t $RUN_LENGTH 101$t >send2_101.in
		echo SEND 2 0.$t $RUN_LENGTH 102$t >send2_102.in
		simple.sh $b

# extra sleep just in case start up for sends is slow
# then wait the number of seconds that should be required to do the
# sends
		sleep 5

		COUNT=0
		while (( $COUNT < (($t * $RUN_LENGTH)/1000) ))
		do
			COUNT=COUNT+1
			sleep 1
		done
		echo $FILE1 
		echo $FILE2
		echo v1v2$b$t.dat
		cp $FILE1 v1v2/$b.$t.dat
		cp $FILE2 v2v1/$b.$t.dat
		killall nc
		sleep 1
	done
done
		
