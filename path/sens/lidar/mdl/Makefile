# Makefile for MDL ILM150-HR library and example code.
#
# Copyright (c) 2009   Regents of the University of California

include $(CAPATH_MK_DEFS) 

PATH_CFLAGS += -g -pg -Wall 

HDRS = mdl.h

EXEC = $(OBJDIR)/mdl

all: $(OBJDIR) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/mdl:	$(HDRS) $(OBJDIR)/mdl.o 

clean:
	rm -f $(OBJDIR)/*.o *.err $(OBJDIR)/*.a $(EXEC)

install:
	cp $(EXEC) ../test
