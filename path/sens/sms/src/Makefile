# Makefile
#
# Copyright (c) 2006   Regents of the University of California
#
include $(CAPATH_MK_DEFS)

HDRS = smsparse.h sms_lib.h

SMS_LIB = $(OBJDIR)/libsms.a
SMS_OBJS = $(OBJDIR)/smsparse_lib.o $(OBJDIR)/sms_lib.o

PATH_LIBS += -lsms
PATH_LDFLAGS += -L$(OBJDIR)

PATH_CFLAGS += -g

EXEC =  $(OBJDIR)/smsparse $(OBJDIR)/smscmd
	
all: 	$(OBJDIR) $(SMS_LIB) $(EXEC)	

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(SMS_LIB): $(SMS_OBJS)
	$(PATH_AR) $(PATH_AR_FLAGS) $@ $(SMS_OBJS)
	cp $@ $(SENS_LIB_DIR)/$(OBJDIR)

$(OBJDIR)/smsparse: $(OBJDIR)/smsparse.o
$(OBJDIR)/smscmd: $(OBJDIR)/smscmd.o

clean: 
	\rm -f *.o $(OBJDIR)/*.o $(OBJDIR)/*.err $(OBJDIR)/*.a *.*~ $(EXEC)

install:
	mkdir -p ../test
	\cp $(EXEC) ../test

# DO NOT DELETE
