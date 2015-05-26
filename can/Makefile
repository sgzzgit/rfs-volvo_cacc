# Top level Makefile for CAN code.
include $(CAPATH_MK_DEFS)

all:
	make -C drv_i82527
	make -C drv_sja1000
	make clean -C drv_common DEVNAME=drv_i82527
	make -C drv_common DEVNAME=drv_i82527
	make install -C drv_common DEVNAME=drv_i82527
	make clean -C drv_common DEVNAME=drv_sja1000
	make -C drv_common DEVNAME=drv_sja1000
	make install -C drv_common DEVNAME=drv_sja1000
	make -C client
	make -C jbus
	echo 'Run setuid.sh as root'

clean:
	make clean -C drv_i82527
	make clean -C drv_sja1000
	make clean -C drv_common DEVNAME=drv_i82527
	make clean -C drv_common DEVNAME=drv_sja1000
	make clean -C client
	make clean -C jbus

