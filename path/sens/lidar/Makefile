# Makefile - PATH IBEO lidar library, including CAN message
# parsing and overall receive of bundled CAN messages over Ethernet
#
# Copyright (c) 2008   Regents of the University of California
#

include $(CAPATH_MK_DEFS)

all: ibeo_can/$(OBJDIR)/libibeo_utils.a 
	(cd ibeo_eth; make)
	(cd denso; make)
	(cd mdl; make)

ibeo_can/$(OBJDIR)/libibeo_utils.a:
	(cd ibeo_can; make)

clean:
	(cd ibeo_can; make clean)
	(cd ibeo_eth; make clean)
