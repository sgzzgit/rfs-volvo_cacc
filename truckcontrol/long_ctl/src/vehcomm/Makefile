# Makefile
#
# Copyright (c) 2008   Regents of the University of California
#

include $(CAPATH_MK_DEFS)

TRUCK_DIR = $(DISTRIB_DIR)/truckcontrol

EXEC =  $(OBJDIR)/veh_rcv $(OBJDIR)/veh_snd $(OBJDIR)/veh_snd_test

all:    $(OBJDIR) $(PATH_OBJS) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/veh_rcv: $(OBJDIR)/veh_rcv.o
$(OBJDIR)/veh_snd: $(OBJDIR)/veh_snd.o

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.err *.*~ $(EXEC)

install:
	cp $(EXEC) $(TRUCK_DIR)/test

# DO NOT DELETE

