# Makefile
#
# Copyright (c) 2008   Regents of the University of California
#

include $(CAPATH_MK_DEFS)

TRUCK_DIR = $(DISTRIB_DIR)/truckcontrol
ASNDIR = ./BSM_asn1_dir
J2735DIR = ./dsrc/asn1j2735
J2735INC = $(J2735DIR)/include

PATH_CFLAGS += -I$(ASNDIR) -I$(J2735INC)
PATH_LDFLAGS += -L$(ASNDIR)
PATH_OBJS += $(ASNDIR)/*.o $(OBJDIR)/veh_lib.o

EXEC =  $(OBJDIR)/veh_rcv $(OBJDIR)/veh_snd $(OBJDIR)/veh_snd_test

all:    $(OBJDIR) $(PATH_OBJS) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/veh_rcv: $(OBJDIR)/veh_rcv.o
$(OBJDIR)/veh_snd: $(OBJDIR)/veh_snd.o
$(ASNDIR)/*.o:
	make -C $(ASNDIR)
clean:
	rm -f $(ASNDIR)/*.o $(OBJDIR)/*.o $(OBJDIR)/*.err *.*~ $(EXEC)

install:
	cp $(EXEC) $(TRUCK_DIR)/test
	cp ./BSM_asn1_dir/BasicSafetyMessage-Volvo.txt .

# DO NOT DELETE

