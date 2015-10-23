# Makefile for EVT 300 radar library and example code.
#
# Copyright (c) 2006   Regents of the University of California

include $(CAPATH_MK_DEFS) 

HDRS = evt300.h
EVT300_LIB = $(OBJDIR)/libevt300.a
EVT300_OBJS = $(OBJDIR)/evt300_lib.o $(OBJDIR)/evt300_msg_list.o 
PATH_LIBS += -levt300

EXEC = $(OBJDIR)/evt300 $(OBJDIR)/evt300_print_list 

all: $(OBJDIR) $(EVT300_LIB) $(EXEC)

$(EVT300_LIB): $(EVT300_OBJS)
	$(PATH_AR) $(PATH_AR_FLAGS) $@ $(EVT300_OBJS)
	cp $(EVT300_LIB) $(SENS_LIB_DIR)/$(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/evt300:	$(OBJDIR)/evt300.o 
$(OBJDIR)/evt300_print_list: $(OBJDIR)/evt300_print_list.o 

clean:
	rm -f $(OBJDIR)/*.o *.err $(OBJDIR)/*.a $(EXEC)

install:
	cp $(EXEC) ../test
