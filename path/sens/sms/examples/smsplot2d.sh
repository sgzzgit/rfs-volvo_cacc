#!/bin/sh

echo "blah=\"$1\"; set term x11 persist; plot blah using 4:5" > simple2D

if [[ $2 == "-jpeg" ]] ; then
	# To make jpeg splot
	echo "set terminal jpeg" >> simple2D
	echo "set output \"$1_simple2D.jpg\"" >> simple2D
	echo "set yrange [-350:150]" >> simple2D
	echo "set xrange [-180:50]" >> simple2D
	echo "plot blah using 4:5" >> simple2D
fi

gnuplot simple2D
rm simple2D

# script to make individual output files for each SMS id
idstr=`awk '{print $3}' $1 | uniq`
for i in $idstr
do
        awk '{if ($3=='$i') print $0}' $1 >$1objID.$i.dat
done

rm objID2D
touch objID2D
for i in $1objID.*.dat
do
	echo \"$i\" 'using 4:5 title 3, \' >> objID2D
done

echo "set yrange [-350:150]" > objID2D.mod
echo "set xrange [-180:50]" >> objID2D.mod
echo 'set term x11 persist; plot \' >> objID2D.mod

# This gets rid of the final comma and backslash
cat objID2D | sed '{$ s/, \\//}' >> objID2D.mod

if [[ $2 == "-jpeg" ]] ; then
	# To make jpeg splot
	echo "set terminal jpeg" >> objID2D.mod
	echo "set output \"$1_objID2D.jpg\"" >> objID2D.mod
	echo "set yrange [-350:150]" >> objID2D.mod
	echo "set xrange [-180:50]" >> objID2D.mod
	echo 'plot \' >> objID2D.mod
	cat objID2D | sed '{$ s/, \\//}' >> objID2D.mod
fi

gnuplot objID2D.mod
rm objID2D* $1objID.*.dat
