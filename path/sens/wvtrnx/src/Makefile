# Makefile, update for portability
#
# Copyright (c) 2006   Regents of the University of California
#
include $(CAPATH_MK_DEFS)

HDRS = wvtrnx.h
WVTRNX_LIB = $(OBJDIR)/libwvtrnx.a
WVTRNX_OBJS = $(OBJDIR)/wvtrnx_lib.o
PATH_LIBS += -lwvtrnx

PATH_CFLAGS += -g

EXEC =  $(OBJDIR)/wvtrnx $(OBJDIR)/wvtrnx_create $(OBJDIR)/wvtrnx_wrfile	

all: 	$(OBJDIR) $(WVTRNX_LIB) $(EXEC)	

$(WVTRNX_LIB): $(WVTRNX_OBJS)
	$(PATH_AR) $(PATH_AR_FLAGS) $@ $(WVTRNX_OBJS)
	cp $(WVTRNX_LIB) $(SENS_LIB_DIR)/$(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/wvtrnx: $(OBJDIR)/wvtrnx.o
$(OBJDIR)/wvtrnx_create: $(OBJDIR)/wvtrnx_create.o
$(OBJDIR)/wvtrnx_wrfile: $(OBJDIR)/wvtrnx_wrfile.o

clean: 
	\rm -f $(OBJDIR)/*.o $(OBJDIR)/*.err *.*~ $(EXEC)

install:
	\cp $(EXEC) ../test

# DO NOT DELETE
