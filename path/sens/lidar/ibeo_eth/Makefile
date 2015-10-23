#	Makefile for Linux code using IBEO Ethernet interface, not
#	CAN, although Ethernet data contains embedded CAN message formats 
#
# Copyright (c) 2008   Regents of the University of California
#
include $(CAPATH_MK_DEFS)

IBEO_DIR=../ibeo_can
PATH_LIBS= -libeo_utils -ldb_clts -llocals 
HDRS = $(IBEO_DIR)/ibeo.h 

PATH_CFLAGS += -I$(CAN_DIR)/include -I$(CAN_DIR)/client

EXEC = $(OBJDIR)/ibeo_dump

all: $(OBJDIR) $(OBJS) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/ibeo_dump: $(IBEO_DIR)/$(OBJDIR)/libibeo_utils.a $(OBJDIR)/ibeo_dump.o 

clean:
	rm -f $(OBJDIR)/*.o *.err *.a $(EXEC)

