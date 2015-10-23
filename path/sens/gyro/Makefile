# Makefile
#
# Copyright (c) 2006-9   Regents of the University of California
#
include $(CAPATH_MK_DEFS)

PATH_CFLAGS += -g -Wall

PATH_HDRS += gyro.h

EXEC =  $(OBJDIR)/gyroread

all: 	$(OBJDIR) $(EXEC)	

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	\rm -f $(OBJDIR)/*.o $(OBJDIR)/*.err *.*~ $(EXEC)

install:
	\cp $(EXEC) ../test

