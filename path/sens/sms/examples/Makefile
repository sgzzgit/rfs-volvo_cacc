# Makefile
#
# Copyright (c) 2006   Regents of the University of California
#
include $(CAPATH_MK_DEFS)

HDRS = smsparse.h

SMS_LIB = ../src/$(OBJDIR)/libsms.a
SMS_OBJS = ../src/$(OBJDIR)/smsparse_lib.o

PATH_LIBS += -lsms
PATH_LDFLAGS += -L../src/$(OBJDIR) 

EXEC =  $(OBJDIR)/readsmsgps
	
all: 	$(OBJDIR) $(EXEC)	

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/readsmsgps: $(OBJDIR)/readsmsgps.o

clean: 
	\rm -f *.o $(OBJDIR)/*.o $(OBJDIR)/*.err $(OBJDIR)/*.a *.*~ $(EXEC)

install:
	\cp $(EXEC) ../test

# DO NOT DELETE
