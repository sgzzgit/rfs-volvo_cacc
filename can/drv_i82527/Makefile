#\file Makefile for QNX6 DAS (Data Acquisition System) resource managers
# Re written for PATH build environment, and to use gcc (no license)
# instead of qcc (Momentics license required) on QNX6 
#
# Copyright (c) 2003-2009   Regents of the University of California
#
#
#

include $(CAPATH_MK_DEFS)

DEVNAME = drv_i82527

CAN_HDRS = $(CAN_DIR)/drv_common/can_man.h \
	$(CAN_DIR)/include/can_defs.h can_mm.h

CAN_OBJS = $(OBJDIR)/can_dev.o 

CAN_DEV_LIB = $(OBJDIR)/libcan_dev.a

PATH_CFLAGS += -I$(CAN_DIR)/include -I$(CAN_DIR)/drv_common

EXECS = $(OBJDIR)/read_can \
	$(OBJDIR)/write_can \
	$(OBJDIR)/search_can \
	$(OBJDIR)/dev_init \
	$(OBJDIR)/read_can_mm \
	$(OBJDIR)/write_can_mm \
	$(OBJDIR)/search_can_mm

all: $(OBJDIR) $(CAN_DEV_LIB) $(EXECS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/libcan_dev.a: $(CAN_OBJS)                                
	$(PATH_AR) $(PATH_AR_FLAGS)  $@ $(CAN_OBJS)    


# install puts executables for this chipset in the can/test directory
# can_man is actually built in drv_common
install:
	cp $(EXECS) $(OBJDIR)/can_man ../test

clean:
	\rm -f $(EXECS) $(OBJDIR)/*.o *.err

