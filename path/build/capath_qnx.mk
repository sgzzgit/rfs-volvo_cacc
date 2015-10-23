# common definitions for PATH builds on QNX6 systems 
#
# Copyright (c) 2008   Regents of the University of California
#

DISTRIB_DIR = /home
SYS_OS_HEADER_FILE = $(DISTRIB_DIR)/path/local/sys_qnx6.h
OBJDIR = qnx
RELEASE_DIR = $(DISTRIB_DIR)/release

CAPATH_DIR = $(DISTRIB_DIR)/path
LOCAL_DIR = $(CAPATH_DIR)/local
DB_DIR = $(CAPATH_DIR)/db
CAN_DIR = $(DISTRIB_DIR)/can

# parts of sens library not yet ported to QNX
# but harmless to include definition as extra search area 
SENS_LIB_DIR = $(CAPATH_DIR)/sens/lib
SENS_INC_DIR = $(CAPATH_DIR)/sens/include

PATH_CC = gcc
PATH_CFLAGS += -c -D_GNU_SOURCE 
PATH_CFLAGS += -I. -I$(LOCAL_DIR) -I$(DB_DIR) -I$(SENS_INC_DIR) \
		 -I/usr/local/include
PATH_CFLAGS += -Wall
PATH_CFLAGS += -g
# Is this still necessary with gcc on QNX6.3? Seems not to work.
# It was used on QNX 6.2.
#PATH_CFLAGS += -V gcc_ntox86

PATH_AR = ar
PATH_LINK = gcc
PATH_LDFLAGS += -L$(LOCAL_DIR)/$(OBJDIR)  -L$(DB_DIR)/$(OBJDIR) \
		-L$(SENS_LIB_DIR)/$(OBJDIR)
PATH_LDFLAGS += -L/lib -L/usr/qnx632/target/qnx6/x86/lib 
PATH_LDFLAGS += -L/usr/qnx632/host/qnx6/x86/usr/lib/gcc-lib/ntox86/2.95.3
# core libs defined so that if required a new library
# can be added to the beginning to redefine PATH_LIBS
PATH_CORE_LIBS = -ldb_clts -llocals -lsocket -lm -lc -lgcc
PATH_LIBS = $(PATH_CORE_LIBS)
PATH_AR_FLAGS = -rv
# PATH_OBJS can be set up in individual Makefiles as desired
PATH_OBJS =


$(OBJDIR)/%.o: %.c $(HDRS)	
	$(PATH_CC) $(CFLAGS) $(PATH_CFLAGS) $*.c -o $@ 

.o:
	$(PATH_LINK) $(LDFLAGS) $(PATH_LDFLAGS) -o $@ $@.o $(PATH_OBJS) $(PATH_LIBS) 
