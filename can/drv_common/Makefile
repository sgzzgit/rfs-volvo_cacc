#\file Makefile for QNX6 CAN resource managers
# Re written for PATH build environment, and to use gcc (no license)
# instead of qcc (Momentics license required) on QNX6 
#
# Copyright (c) 2003-2009   Regents of the University of California

include $(CAPATH_MK_DEFS)

# DEVNAME must be set; top-level Makefile does both with appropriate
# install, or can uncomment while making changes in only one
# of the device directories 
#DEVNAME=drv_i82527
#DEVNAME=drv_sja100
DEVDIR=$(CAN_DIR)/$(DEVNAME)

CAN_OBJS = $(OBJDIR)/can_init.o \
	$(OBJDIR)/can_if.o \
	$(OBJDIR)/io_func.o  

CAN_HDRS = $(DEVDIR)/can_mm.h \
	$(CAN_DIR)/include/can_defs.h \
	can_man.h 

PATH_CFLAGS += -I$(CAN_DIR)/include -I$(DEVDIR)
PATH_OBJS += $(CAN_OBJS)
PATH_LDFLAGS += -L$(DEVDIR)/$(OBJDIR)
PATH_LIBS += -lcan_dev

EXECS = $(OBJDIR)/can_man 

all: $(OBJDIR) $(CAN_OBJS) $(EXECS) 

$(OBJDIR):
	mkdir -p $(OBJDIR)

install: 
	cp $(EXECS) $(DEVDIR)/$(OBJDIR)

clean:
	\rm -f $(EXECS) $(DEVDIR)/$(EXECS) $(OBJDIR)/*.o *.err

