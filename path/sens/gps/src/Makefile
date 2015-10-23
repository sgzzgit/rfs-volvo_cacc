# Makefile - PATH GPS library 
#
# Copyright (c) 2008   Regents of the University of California
#
include $(CAPATH_MK_DEFS)

#PATH_CFLAGS += -g
PATH_OBJS = $(OBJDIR)/path_gps_lib.o $(OBJDIR)/enu2lla.o

HDRS= path_gps_lib.h

EXEC = $(OBJDIR)/libpath_gps.a 


all: $(OBJDIR) $(PATH_OBJS) $(EXEC)

$(OBJDIR)/libpath_gps.a: $(HDRS) $(PATH_OBJS)
	$(PATH_AR) $(PATH_AR_FLAGS)  $@ $(PATH_OBJS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

install:
	cp $(EXEC) $(SENS_LIB_DIR)/$(OBJDIR) 

clean:
	rm -f $(OBJDIR)/*.o *.err $(EXEC)
