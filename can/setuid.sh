#!/bin/sh
TOP_DIR=`pwd`

cd $TOP_DIR/drv_i82527/qnx
for i in can_man read_can write_can search_can read_can_mm write_can_mm search_can_mm
do
	chown root $i
	chmod 4775 $i
done

cd $TOP_DIR/drv_sja1000/qnx
for i in can_man regs_sja1000 read_can_mm write_can_mm
do
	chown root $i
	chmod 4775 $i
done


