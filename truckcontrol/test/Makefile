# Makefile to copy in executables that are run by simple menu (.sm) scripts
# from the TEST_DIR, and come from directories in the PATH tree.
#
# Executables from the truckcontrol tree are copied here as part of 
# make installs local to those directories.
#
# CAN executables are invoked directly from their build directories, because
# of the possible name confusion between I82527 and SJA1000 drivers
# and between path_can and steinhoff clients.
#
# Script also cleans out all executables from TEST_DIR, as well as
# and .log, .tmg or .dbg files created locally.

include $(CAPATH_MK_DEFS)

HOME_DIR=$(DISTRIB_DIR)/path
TEST_DIR=.

PATH_EXECS= db_slv evt300 rdlidar dmm32 rddmm32 gpsrcv gpsdb
TRUCK_EXECS= veh_trk long_trk veh_snd veh_rcv long_out \
		jbussendGPS jbussend cmdtest rdbrake test_utils trk_io \
		set_fault setLEDs rdswitches trk_cr trk_wr \
		print_long_in veh_snd_test dvi_snd dvi_rcv

install:
	cp $(HOME_DIR)/db/qnx/db_slv $(TEST_DIR)
	cp $(HOME_DIR)/sens/gps/examples/qnx/gpsrcv $(TEST_DIR) 
	cp $(HOME_DIR)/sens/gps/examples/qnx/gpsdb $(TEST_DIR) 
#	cp $(HOME_DIR)/sens/radar/qnx/evt300 $(TEST_DIR) 
#	cp $(HOME_DIR)/sens/lidar/denso/qnx/rdlidar $(TEST_DIR) 
#	cp $(HOME_DIR)/sens/lidar/mdl/qnx/mdl $(TEST_DIR) 
#	cp $(HOME_DIR)/sens/das/driver/qnx/dmm32 $(TEST_DIR) 

clean:
	rm -f *.dbg *.tmg *.log 
	rm -f $(PATH_EXECS)
	rm -f $(TRUCK_EXECS)
