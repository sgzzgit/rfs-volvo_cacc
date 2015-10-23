#!/bin/sh

lnx/rt_mtr >mcnu/t$1.m$2.dat &
lnx/rt_snd -a 192.168.255.255  -b -v -t $1 -n 1000 -m $2 &
