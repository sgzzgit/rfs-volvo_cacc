#!/bin/sh
#\file
# time_clt millisec trials variable 

	test_update -t $1 -n $2 -v $3 &
	test_read  -t $1 -n $2 -v $3 > test.$1.$2 
	echo "finished $2 trials, $1 millisecond interval"	
