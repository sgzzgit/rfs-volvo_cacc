# DISTRIB_DIR is assumed to be a subdirectory of the main vehicle directory


include $(CAPATH_MK_DEFS)

# Location of files common to all projects under long_ctl/src
TRUCKDIR = $(DISTRIB_DIR)/truckcontrol
COMMON_DIR = $(TRUCKDIR)/long_ctl/src/common
JBUS_DIR = $(CAN_DIR)/jbus
AVCS_DIR = $(TRUCKDIR)/long_ctl/src/avcs
COMM_DIR = $(TRUCKDIR)/long_ctl/src/vehcomm

CFLAGS = $(PATH_CFLAGS) -I$(COMMON_DIR) -I$(JBUS_DIR)/include -DPROTOTYPE
CFLAGS += -I$(AVCS_DIR)
CFLAGS += -I$(COMM_DIR)

LDFLAGS = $(PATH_LDFLAGS) -L$(DISTRIB_DIR)/local/$(OBJDIR) -L$(COMMON_DIR) -L. -L$(JBUS_DIR)/lib

LINK = $(PATH_LINK)
LONG_EXECS = test_utils long_out long_ctl
LONG_LIB = liblong_utils.a
LIBS = -llong_utils -ljbus $(PATH_LIBS) 

#
# User Defined macros for files, directories, etc.
#

# Location/name of executable
TARGET = $(OBJDIR)/cmdtest

# Files from this directory to include in executable
LONG_CTL_SRCS = cmdtasks.c
LONG_CTL_OBJS = $(LONG_CTL_SRCS:c=o)

#
# Default rules
#
 
all:  $(TARGET)

$(TARGET): long_ctl.o $(LONG_CTL_OBJS) 
	if [[ ! -d $(OBJDIR) ]] ; then mkdir $(OBJDIR); fi
	$(LINK) $(LDFLAGS) -o $@ $^ $(LIBS)
	/bin/rm -f *.o 

#
# Object file dependencies
#

long_ctl.o:	$(COMMON_DIR)/long_ctl.c $(COMMON_DIR)/long_ctl.h
	$(CC) $(CFLAGS) -o $@ $<


$(LONG_CTL_OBJS):	$(COMMON_DIR)/long_ctl.h

#
# Distribution control rules
#

install: all
	cp -f $(TARGET) ../test
	cp -f $(TARGET) $(TRUCKDIR)/test

clean:
	/bin/rm -f *.o \
	*.err \
	$(TARGET)

distclean: clean
	cd $(DISTRIB_DIR)/long_ctl/test; /bin/rm -f cmdtest
