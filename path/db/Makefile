# Makefile for in-memory data server
#
# CAPATH_MK_DEFS must be set in the environment to correct file of
# definitions for the compile environment (lnx, qnx or sobos) (check
# in /home/path/build for an appropriate makefile)
#
# Copyright (c) 2006   Regents of the University of California
#

HDRS = db_lib.h db_slv.h db_comm.h db_clt.h db_utils.h path_datahub.h

include $(CAPATH_MK_DEFS)

DB_OBJS = $(OBJDIR)/db_clt.o $(OBJDIR)/db_lib.o \
		$(OBJDIR)/db_comm.o $(OBJDIR)/db_utils.o
DB_CPP_OBJS = $(OBJDIR)/path_datahub.o

EXEC = $(OBJDIR)/libdb_clts.a  $(OBJDIR)/db_slv $(OBJDIR)/libpathcpp.a

#CFLAGS += -g

$(OBJDIR)/%.o: %.cpp $(DB_HDRS) $(CPP_HDRS)
	$(PATH_CC) $(CFLAGS) $(PATH_CFLAGS) -c $*.cpp -o $@ 

all: $(OBJDIR) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/db_slv:	$(OBJDIR)/db_slv.o 

$(OBJDIR)/libdb_clts.a: $(DB_OBJS) 
	$(PATH_AR) $(PATH_AR_FLAGS)  $@ $(DB_OBJS) 
$(OBJDIR)/libpathcpp.a: $(DB_CPP_OBJS) 
	$(PATH_AR) $(PATH_AR_FLAGS)  $@ $(DB_CPP_OBJS) 

clean:
	rm -f *.o *.err *.a $(EXEC)
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.err $(OBJDIR)/*.a  $(EXEC)
