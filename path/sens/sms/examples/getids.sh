#!/bin/sh
# script to make individual output files for each SMS id
idstr=`awk '{print $3}' sms.out | uniq`
for i in $idstr
do
        awk '{if ($3=='$i') print $0}' sms.out >smsdata/sms$i.out
done

