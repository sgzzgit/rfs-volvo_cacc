#\file Makefile for PATH CAN client library and example program
#
# Copyright (c) 2008   Regents of the University of California
#
# In order to use the default rules in the included $(CAPATH_MK_DEFS)
# file, symbolic links are created with the code files in the parent
# directory. 

include $(CAPATH_MK_DEFS)

HDRS = can_client.h $(CAN_DIR)/include/can_defs.h

LOCAL_LIB = $(OBJDIR)/libcapath_can.a

EXECS = $(OBJDIR)/can_tx $(OBJDIR)/can_rx $(OBJDIR)/can_rx2 $(OBJDIR)/can_rx3\
	$(OBJDIR)/vaa_sim_sens_rx \
	$(OBJDIR)/vaa_raw_sens_rx \
	$(OBJDIR)/can_rx4 $(OBJDIR)/can_rx5 $(OBJDIR)/can_set_config \
	$(OBJDIR)/can_print_cf $(OBJDIR)/can_print_stat

CAN_OBJS = $(OBJDIR)/can_client_path_can.o

PATH_CFLAGS += -g -I$(CAN_DIR)/include -I$(CAN_DIR)

PATH_LIBS += -lcapath_can 

PATH_LDFLAGS += -L$(CAN_DIR)/client/path_can/$(OBJDIR) 

all: $(OBJDIR) $(CAN_OBJS) $(LOCAL_LIB) $(EXECS) 

$(OBJDIR):
	mkdir -p $(OBJDIR)
	$(CAN_DIR)/client/mklinks $(CAN_DIR)

$(OBJDIR)/libcapath_can.a: $(CAN_OBJS)                                
	$(PATH_AR) $(PATH_AR_FLAGS)  $@ $(CAN_OBJS)    

install: all
	cp $(EXECS) $(CAN_DIR)/dachs_sja/test 

clean:
	\rm -f $(LOCAL_LIB) $(EXECS) $(OBJDIR)/*.o $(OBJDIR)/*.err

