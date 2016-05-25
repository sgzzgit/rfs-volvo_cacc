# common definitions for PATH builds on Linux systems 
#
# These definitions can be customized for different checkout
# patterns and for different compile environments.
# They are referenced within the Makefiles through the
# CAPATH_MK_DEFS environment variable, which must be
# set correclty for your compile environment.
#
# PATH_CFLAGS and PATH_LDFLAGS are set up for "path" tree.
# Some definitions are included for "atsc" and "cicas" trees
# as well, since they are referenced by many programs developed
# for the CICAS project, but these definitions are not included
# in the default definitions of PATH_CFLAGS and PATH_LDFLAGS
# given below in order to allow these definitions to be used
# by other projects that use the sensor libraries but not
# the traffic signal and intersection functionality. 
# Alterations to PATH_CFLAGS and PATH_LDFLAGS to include
# these CICAS and ATSC definitions must be in the individual
# Makefiles.
#
# Copyright (c) 2008   Regents of the University of California
#

# Changing this definition of the distribution directory allows
# the tree to be checked out and built under any directory.
#DISTRIB_DIR = /home/capath
DISTRIB_DIR = /home
# This is the file that will be linked to sys_os.h by the Makefile in
# the path/local directory
SYS_OS_HEADER_FILE = sys_linux.h
OBJDIR = lnx
RELEASE_DIR = $(DISTRIB_DIR)/release

# Shared utilty and sensor code directories
CAPATH_DIR = $(DISTRIB_DIR)/path
LOCAL_DIR = $(CAPATH_DIR)/local
DB_DIR = $(CAPATH_DIR)/db
SENS_LIB_DIR = $(CAPATH_DIR)/sens/lib
SENS_INC_DIR = $(CAPATH_DIR)/sens/include
# PATH utilities for Tilcon display package 
TILC_DIR = $(CAPATH_DIR)/tilcon
TILC_PATH_LIB = $(TILC)/$(OBJDIR)

# Not all programs need these definitions, may be added
TILCON_LIB_DIR = /usr/Tilcon/Lib
TILCON_INC_DIR = /usr/Tilcon/Include
TILCON_CFLAGS = -I$(TILC_DIR) -I$(TILCON_INC_DIR) -DCC_TRT_LINUX
TILCON_LDFLAGS = -L$(TILCON_LIB_DIR) -L$(TILC_DIR)/$(OBJDIR)
TILCON_LIBS =  -ltilc -lTRTAPI

# Actuated Traffic Signal Controller code directories
ATSC_DIR = $(DISTRIB_DIR)/atsc
ATSC_INC_DIR = $(ATSC_DIR)/include
ATSC_LIB_DIR = $(ATSC_DIR)/lib
ATSC_IX_DIR = $(ATSC_DIR)/ix

# CICAS project directories
CICAS_DIR = $(DISTRIB_DIR)/cicas
CICAS_SRC_DIR = $(CICAS_DIR)/src
CICAS_EXP_DIR = $(CICAS_DIR)/exp
CICAS_FUSION_DIR = $(CICAS_DIR)/fusion
CICAS_VEH_DIR = $(CICAS_DIR)/veh

# Transit Signal Priority code directories
TSP_DIR = $(DISTRIB_DIR)/tsp
TSP_INC_DIR = $(TSP_DIR)/include

# CICAS_TSA project directories
CICASTSA_DIR = $(DISTRIB_DIR)/cicas_tsa
CICASTSA_INC_DIR = $(CICASTSA_DIR)/include
 
# Video directories
VIDEO_DIR = /home/capath/video
PATH_CC = gcc
PATH_CPP = g++
PATH_CFLAGS += -c -D_GNU_SOURCE -Wall
PATH_CFLAGS += -I. -I$(LOCAL_DIR) -I$(DB_DIR) -I$(SENS_INC_DIR) 
PATH_CFLAGS += -I/usr/local/include

# by default, debugging is not turned on; can turn on here 
#PATH_CFLAGS = -g

PATH_AR = ar
PATH_LINK = gcc
PATH_LINKPP = g++
PATH_LDFLAGS += -L$(LOCAL_DIR)/$(OBJDIR)  -L$(DB_DIR)/$(OBJDIR) -L$(SENS_LIB_DIR)/$(OBJDIR)
# PATH_CORE_LIBS can be used to redefine longer PATH_LIBS with libraries
# at either end
PATH_CORE_LIBS = -ldb_clts -llocals -lrt -lm
PATH_LIBS = $(PATH_CORE_LIBS)
PATH_AR_FLAGS = -rv
# PATH_OBJS can be set up in individual Makefiles as desired
PATH_OBJS =

$(OBJDIR)/%.o: %.c $(HDRS)	
	$(PATH_CC) $(CFLAGS) $(PATH_CFLAGS) $*.c -o $@ 

$(OBJDIR)/%.o: %.cpp $(HDRS)	
	$(PATH_CPP) $(CFLAGS) $(PATH_CFLAGS) $*.cpp -o $@ 

.o:	$(PATH_OBJS)
	$(PATH_LINK) $(LDFLAGS) $(PATH_LDFLAGS) -o $@ $@.o $(PATH_OBJS) $(PATH_LIBS) 
