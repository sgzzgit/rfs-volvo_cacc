# Makefile
#
# Copyright (c) 2008   Regents of the University of California
#

include $(CAPATH_MK_DEFS)

TRUCK_DIR = $(DISTRIB_DIR)/truckcontrol
LONG_SRC_DIR = $(TRUCK_DIR)/long_ctl/src
COMMON_DIR = $(LONG_SRC_DIR)/common 
VEHCOMM_DIR = $(LONG_SRC_DIR)/vehcomm 
J1939_HEADER_DIR = $(CAN_DIR)/jbus/include
J1939_LIB_DIR = $(CAN_DIR)/jbus/lib
CAN_CLIENT_LIB_DIR = $(CAN_DIR)/client/path_can/qnx
CAN_DEFS_INCLUDE_DIR = $(CAN_DIR)/include
CAN_CLIENT_INCLUDE_DIR = $(CAN_DIR)/client

LONG_LIBS = -llong_utils -ljbus

PATH_CFLAGS += -I$(J1939_HEADER_DIR) -I$(COMMON_DIR) -I$(VEHCOMM_DIR) -I$(CAN_DEFS_INCLUDE_DIR) -I$(CAN_CLIENT_INCLUDE_DIR)

PATH_LDFLAGS += -L$(COMMON_DIR) -L$(J1939_LIB_DIR) -L$(CAN_CLIENT_LIB_DIR)

PATH_LIBS = $(LONG_LIBS) $(PATH_CORE_LIBS) -lcapath_can 

PATH_OBJS += $(OBJDIR)/trk_table.o

EXEC =  $(OBJDIR)/trk_wr \
	$(OBJDIR)/trk_cr \
	$(OBJDIR)/rdswitches\
	$(OBJDIR)/replay_truck\
	$(OBJDIR)/setLEDs\
	$(OBJDIR)/set_fault\
	$(OBJDIR)/jbussend\
	$(OBJDIR)/jbussendGPS\
	$(OBJDIR)/fake_switches

all: $(OBJDIR) $(PATH_OBJS) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/veh_trk: $(OBJDIR)/veh_trk.o

clean:
	rm -f $(OBJDIR)/*.o $(EXEC) 

install:
	cp $(EXEC) $(TRUCK_DIR)/test 
