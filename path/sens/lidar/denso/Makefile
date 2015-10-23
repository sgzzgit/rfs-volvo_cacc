# Makefile for MDL ILM150-HR library and example code.
#
# Copyright (c) 2009   Regents of the University of California

include $(CAPATH_MK_DEFS) 

JBUS_DIR = $(CAN_DIR)/jbus/include 

PATH_CFLAGS += -g -pg -Wall -I$(JBUS_DIR)

HDRS = densolidar.h

EXEC = $(OBJDIR)/rdlidar $(OBJDIR)/wrlidar

all: $(OBJDIR) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)


clean:
	rm -f $(OBJDIR)/*.o *.err $(OBJDIR)/*.a $(EXEC)

install:
	cp $(EXEC) ../test
