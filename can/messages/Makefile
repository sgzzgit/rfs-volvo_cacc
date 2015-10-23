# Makefile to test message code using include file in $(DISTRIB_DIR)/path/build 
#
include $(CAPATH_MK_DEFS)

PATH_CFLAGS += -I. -I$(CAN_DIR)/client -I$(CAN_DIR)/include
PATH_LIBS += -lcapath_can 
PATH_LDFLAGS += -L$(CAN_DIR)/client/path_can/$(OBJDIR) 

HDRS=messages.h vaa_msg_struct.h msg_pack.h
PATH_OBJS=$(OBJDIR)/vaa_msg_struct.o \
		$(OBJDIR)/msg_pack.o \
		$(OBJDIR)/msg_unpack.o 
EXECS=$(OBJDIR)/test_pack $(OBJDIR)/fuzz_test \
		$(OBJDIR)/cc_hmi_recv \
#		$(OBJDIR)/cc_hmi_send

all: $(OBJDIR) $(PATH_OBJS) $(EXECS) 

$(OBJDIR):
	mkdir -p $(OBJDIR)

test_pack: $(PATH_OBJS) test_pack.o
fuzz_test: $(PATH_OBJS) fuzz_test.o
cc_hmi_recv: $(PATH_OBJS) cc_hmi_recv.o
cc_hmi_send: $(PATH_OBJS) cc_hmi_send.o

clean:
	rm -f *~ $(OBJDIR)/*.o $(EXECS)
