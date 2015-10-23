#!/bin/sh

echo "blah=\"$1\"; set term x11 persist; splot blah using 4:5:3" > barf

if [[ $2 == "-jpeg" ]] ; then
	# To make jpeg splot
	echo "set terminal jpeg" >> barf
	echo "set output \"$13D.jpg\"" >> barf
	echo "set yrange [-350:150]" >> barf
	echo "set xrange [-50:50]" >> barf
	echo "splot blah using 4:5:3" >> barf
fi

gnuplot barf
rm barf
