#!/bin/sh

# script to make individual output files for each SMS id
idstr=`awk '{print $6}' $1 | uniq`
for i in $idstr
do
        awk '{if ($6=='$i') print $0}' $1 >$1_optsens.$i.dat
done
if [ -e optsens ]
then
	rm optsens
fi
touch optsens
for i in $1_optsens.*.dat
do
	for j in $idstr
	do
		if [[ $i == $1_optsens.$j.dat ]]
		then
			echo \"$i\" 'using 10:11 title '\"optsens=$j:\"', \' >> optsens
			break
		fi
	done
done

echo "set yrange [-15:15]" > optsens.mod
echo "set xrange [-15:15]" >> optsens.mod
echo 'set term x11 persist; plot \' >> optsens.mod

# This gets rid of the final comma and backslash
cat optsens | sed '{$ s/, \\//}' >> optsens.mod

if [[ $2 == "-jpeg" ]] ; then
	# To make jpeg plot
	echo "set terminal jpeg" >> optsens.mod
	echo "set output \"$1_optsens.jpg\"" >> optsens.mod
	echo 'plot \' >> optsens.mod
	cat optsens | sed '{$ s/, \\//}' >> optsens.mod
fi

gnuplot optsens.mod
rm optsens* $1_optsens.*.dat
