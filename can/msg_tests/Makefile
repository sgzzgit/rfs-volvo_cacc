#\file Makefile for VAA CAN message formatting and test utilities
#
# Copyright (c) 2008   Regents of the University of California
#
# In order to use the default rules in the included $(CAPATH_MK_DEFS)
# file, symbolic links are created with the code files in the parent
# directory. 

include $(CAPATH_MK_DEFS)

HDRS = msg_packer.h datastructs.h msg_descriptors.h

EXECS = $(OBJDIR)/msg_packer_test $(OBJDIR)/input_gen $(OBJDIR)/read_input \
	$(OBJDIR)/schedule_send $(OBJDIR)/schedule_recv \
	$(OBJDIR)/sensor_sim_send $(OBJDIR)/sensor_sim_recv \
	$(OBJDIR)/control_sensor_sim_send $(OBJDIR)/control_sensor_sim_recv \
	$(OBJDIR)/db_writer

# removing $(OBJDIR)/msg_send from list for now

MSG_OBJS = $(OBJDIR)/msg_packer.o $(OBJDIR)/msg_unpacker.o $(OBJDIR)/msg_descriptors.o

LOCAL_LIB = $(OBJDIR)/libmsg_packer.a

PATH_CFLAGS += -g -I$(CAN_DIR)/include -I$(CAN_DIR) -I$(CAN_DIR)/client

PATH_LIBS += -lcapath_can -lmsg_packer 

PATH_LDFLAGS += -L$(CAN_DIR)/client/path_can/$(OBJDIR) \
		-L$(OBJDIR)

all: $(OBJDIR) $(CAN_OBJS) $(LOCAL_LIB) $(EXECS) 

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/libmsg_packer.a: $(MSG_OBJS)                                
	$(PATH_AR) $(PATH_AR_FLAGS)  $@ $(MSG_OBJS)    

install: all
	cp $(EXECS) $(CAN_DIR)/test 

clean:
	\rm -f $(LOCAL_LIB) $(EXECS) $(OBJDIR)/*.o $(OBJDIR)/*.err

