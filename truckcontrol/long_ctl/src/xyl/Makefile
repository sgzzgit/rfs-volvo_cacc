# Makefile for main longitudinal control directory, authored by Xiao-yun Lu
include $(CAPATH_MK_DEFS)

TRUCKDIR = $(DISTRIB_DIR)/truckcontrol
COMMON_DIR = $(DISTRIB_DIR)/truckcontrol/long_ctl/src/common
JBUS_DIR = $(CAN_DIR)/jbus
VEHCOMM_DIR = $(TRUCKDIR)/long_ctl/src/vehcomm
AVCS_DIR = $(TRUCKDIR)/long_ctl/src/avcs

CFLAGS = $(PATH_CFLAGS) -I$(COMMON_DIR) -I$(JBUS_DIR)/include -DPROTOTYPE
CFLAGS += -I$(VEHCOMM_DIR) -I$(AVCS_DIR)

LINK = $(PATH_LINK)
LINKFLAGS = $(PATH_LDFLAGS) -L$(COMMON_DIR) -L. -L$(JBUS_DIR)/lib
LIBS = -llong_utils -ljbus -ltrack $(PATH_LIBS) 

TRACKLIB = libtrack.a

#
# User Defined macros for files, directories, etc.
#

# Location/name of executable
TARGET = $(OBJDIR)/long_trk

# Files from this directory to include in executable
LONG_CTL_SRCS = long_trk.c long_trk_func.c
LONG_CTL_OBJS = $(LONG_CTL_SRCS:c=o)

#
# Default rules
#

all:   $(TRACKLIB) $(TARGET) 

.c.o:
	$(CC) $(CFLAGS) -o $@ $<

#
# Object file dependencies
#

long_ctl.o:	$(COMMON_DIR)/long_ctl.c $(COMMON_DIR)/long_ctl.h
	$(CC) $(CFLAGS) -o $@ $<

$(TARGET): long_ctl.o $(LONG_CTL_OBJS) 
	if [[ ! -d $(OBJDIR) ]] ; then mkdir -p $(OBJDIR) ; fi
	$(LINK) $(LINKFLAGS) -o $@ $^ $(LIBS)

$(LONG_CTL_OBJS): $(COMMON_DIR)/long_ctl.h track.h long_trk.h

#
# Distribution control rules
#

install:  all
	cp -f $(TARGET) $(TRUCKDIR)/test
#	cp realtime.ini $(TRUCKDIR)/test

clean:
	/bin/rm -f *.o \
	*.err \
	$(TARGET)

distclean: clean
	cd $(DISTRIB_DIR)/long_ctl/test; /bin/rm -f cmdtest

