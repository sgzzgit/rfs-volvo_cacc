# common definitions for PATH cross-compile builds for SOBUs
# (Savari On-Board Units with Sobos operating system)
#
# These definitions can be changed for different checkout
# patterns and for different compile environments.
# They are referenced within the Makefiles through the
# CAPATH_MK_DEFS environment variable, which must be
# set correclty for your compile environemtn.
#
# PATH_CFLAGS and PATH_LDFLAGS are set up for "path" tree
# Some definitions are include for "atsc" and "cicas" trees
# as well, since they are referenced by code in other trees,
# but alterations to PATH_CFLAGS and PATH_LDFLAGS to include
# these definitions must be in the individual Makefiles.
# PATH_CFLAGS, PATH_LDFLAGS, and PATH_LIBS are used in the
# default rules
#
# Copyright (c) 2008   Regents of the University of California
#

DISTRIB_DIR = /home/gems/sdk/sobos/build_dir/i386/capath_util-1.0/
OBJDIR = sobu

# Shared utilty and sensor code directories

CAPATH_DIR = $(DISTRIB_DIR)/path
LOCAL_DIR = $(CAPATH_DIR)/local
DB_DIR = $(CAPATH_DIR)/db
SENS_LIB_DIR = $(CAPATH_DIR)/sens/lib
SENS_INC_DIR = $(CAPATH_DIR)/sens/include
# PATH utilities for Tilcon display package 
TILC_DIR = $(CAPATH_DIR)/tilcon

# Not all programs need these definitions, may be added
TILCON_LIB_DIR = /usr/Tilcon/Lib
TILCON_INC_DIR = /usr/Tilcon/Include
TILCON_CFLAGS = -I$(TILC_DIR) -I$(TILCON_INC_DIR) -DCC_TRT_LINUX
TILCON_LDFLAGS = -L$(TILCON_LIB_DIR) -L$(TILC_DIR)/$(OBJDIR)
TILCON_LIBS =  -ltilc -lTRTAPI

# Actuated Traffic Signal Controller code directories
ATSC_DIR = $(DISTRIB_DIR)/atsc
ATSC_INC_DIR = $(ATSC_DIR)/include
ATSC_IX_DIR = $(ATSC_DIR)/ix

# CICAS project directories
CICAS_DIR = $(DISTRIB_DIR)/cicas
CICAS_SRC_DIR = $(CICAS_DIR)/src
CICAS_EXP_DIR = $(CICAS_DIR)/exp
CICAS_FUSION_DIR = $(CICAS_DIR)/fusion

# Transit Signal Priority code directories
TSP_DIR = $(DISTRIB_DIR)/tsp
TSP_INC_DIR = $(TSP_DIR)/include

## commented out are defined in the Makefile for the SOBU package
## to get the cross-compile tools
##PATH_CC = gcc
##PATH_CPP = g++
##PATH_AR = ar
PATH_CFLAGS += -c -D_GNU_SOURCE 
PATH_CFLAGS += -I. -I$(LOCAL_DIR) -I$(DB_DIR) -I$(SENS_INC_DIR) \
		 -I/usr/local/include
##PATH_LINK = gcc
##PATH_LINKPP = g++
PATH_LDFLAGS += -L$(LOCAL_DIR)/$(OBJDIR)  -L$(DB_DIR)/$(OBJDIR) -L$(SENS_LIB_DIR)/$(OBJDIR)

# used when redefining PATH_LIBS with library at beginning
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
