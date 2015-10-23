#	Makefile for Linux code using IBEO Ethernet interface, not
#	CAN, although Ethernet data contains embedded CAN message formats 
#
# Copyright (c) 2008   Regents of the University of California
#

HDRS = ibeo.h 

include $(CAPATH_MK_DEFS)

PATH_CFLAGS += -I$(CAN_DIR)/include -I$(CAN_DIR)/client

OBJS=$(OBJDIR)/ibeo_utils.o

EXEC = $(OBJDIR)/ibeo_rx_file $(OBJDIR)/libibeo_utils.a

all: $(OBJDIR) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

PATH_LIBS = -libeo_utils -ldb_clts -llocals -lm

$(OBJDIR)/ibeo_rx_file:		$(OBJDIR)/ibeo_rx_file.o $(OBJDIR)/libibeo_utils.a 

$(OBJDIR)/libibeo_utils.a: $(OBJS)
	$(PATH_AR) $(PATH_AR_FLAGS)  $@ $(OBJS)
	cp $@ $(SENS_LIB_DIR)/$(OBJDIR)

clean:
	rm -f $(OBJDIR)/*.o *.err *.a $(EXEC)

