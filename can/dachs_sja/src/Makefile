#\file Makefile for QNX6 DAS (Data Acquisition System) resource managers
#
# Copyright (c) 2008   Regents of the University of California
#
include $(CAPATH_MK_DEFS)

HDRS = can_client.h  ../candefs.h ../canglob.h

#EXECS = $(OBJDIR)/can_tx $(OBJDIR)/can_rx 
EXECS = $(OBJDIR)/recv_user $(OBJDIR)/send_user \
	$(OBJDIR)/recv_user2 $(OBJDIR)/send_user2 

PATH_CFLAGS += -I../include

PATH_LDFLAGS += -L../lib

PATH_LIBS += -lCAN

CAN_OBJS = $(OBJDIR)/can_client.o

PATH_OBJS += $(CAN_OBJS) 

all: $(OBJDIR) $(CAN_OBJS) $(EXECS) 
	cp $(EXECS) ../test

$(OBJDIR):
	mkdir -p $(OBJDIR)

install: all
	cp $(EXECS) ../test

clean:
	\rm -f $(EXECS) *.o *.err

