#\file Makefile for QNX6 Philipps SJA100 Resource Manager resource managers
# Builds device specific library used in drv_common.
#
# Copyright (c) 2003-2009   Regents of the University of California
#

include $(CAPATH_MK_DEFS)

DEVNAME = drv_sja1000

DRV_COMMON = ../drv_common

CAN_OBJS = $(OBJDIR)/sja1000funcs.o $(OBJDIR)/can_dev.o 

CAN_DEV_LIB = $(OBJDIR)/libcan_dev.a

CAN_HDRS = sja1000.h defs.h linux/delay.h can_mm.h\
	$(CAN_DIR)/include/can_defs.h 

PATH_CFLAGS += -I$(CAN_DIR)/include -I$(DRV_COMMON)

EXECS = $(OBJDIR)/read_can_mm \
	$(OBJDIR)/write_can_mm \
	$(OBJDIR)/regs_sja1000 

all: $(OBJDIR) $(CAN_DEV_LIB) $(EXECS) 

$(OBJDIR)/libcan_dev.a: $(CAN_OBJS)                                
	$(PATH_AR) $(PATH_AR_FLAGS)  $@ $(CAN_OBJS)    

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	\rm -f $(EXECS) $(OBJDIR)/*.o $(OBJDIR)/*.a *.err

