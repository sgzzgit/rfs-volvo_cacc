#\file Makefile for PATH CAN client library and example program
#
# Copyright (c) 2008   Regents of the University of California
#

include $(CAPATH_MK_DEFS)

HDRS = $(CAN_DIR)/client/can_client.h $(CAN_DIR)/include/can_defs.h

PATH_OBJS += $(OBJDIR)/vaa_db_utils.o

EXECS = $(OBJDIR)/vaa_input_gen $(OBJDIR)/vaa_db_writer \
	$(OBJDIR)/vaa_db_rx $(OBJDIR)/vaa_db_tx  \
	$(OBJDIR)/vaa_db_create \
	$(OBJDIR)/vaa_db_reader \
	$(OBJDIR)/db_timer_send $(OBJDIR)/db_timer_recv

PATH_CFLAGS += -g -I$(CAN_DIR)/include -I$(CAN_DIR)/client

PATH_LIBS += -lcapath_can 

PATH_LDFLAGS += -L$(CAN_DIR)/client/path_can/$(OBJDIR) 

all: $(OBJDIR) $(PATH_OBJS) $(LOCAL_LIB) $(EXECS) 

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	\rm -f $(LOCAL_LIB) $(EXECS) $(OBJDIR)/*.o $(OBJDIR)/*.err

