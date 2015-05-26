# LONG_DIR is assumed to be a subdirectory of the main vehicle directory
# as is the INCLUDE_DIR directory 
#
# make includes in the main vehicle directory will link shared includes
# from other directories to that directory

include $(CAPATH_MK_DEFS)

# Location of files common to all projects under long_ctl/src
TRUCKDIR = $(DISTRIB_DIR)/truckcontrol
COMMON_DIR = $(TRUCKDIR)/long_ctl/src/common
JBUS_DIR = $(CAN_DIR)/jbus
AVCS_DIR = $(TRUCKDIR)/long_ctl/src/avcs

CCFLAGS = $(PATH_CFLAGS) -I$(COMMON_DIR) -I$(JBUS_DIR)/include -DPROTOTYPE -I$(AVCS_DIR)

LINK = $(PATH_LINK)
LINKFLAGS = $(PATH_LDFLAGS) -L$(COMMON_DIR) -L. -L$(JBUS_DIR)/lib

LONG_EXECS = longprof mkprof sndprof vehprof
LIBS = -llong_utils -ljbus $(PATH_LIBS)
 
all:	$(LONG_EXECS)  	

.c.o:
	$(CC) $(CCFLAGS) -o $@ $<

sndprof:	sndprof.o vehicle_profile.o 
	$(LINK) $(LINKFLAGS) -o $@ sndprof.o vehicle_profile.o  $(LIBS)

mkprof:	mkprof.o 
	$(LINK) $(LINKFLAGS) -o $@ mkprof.o  $(LIBS)

longprof: long_wrprof.o long_out_profile.o
	$(LINK) $(LINKFLAGS) -o $@ long_wrprof.o long_out_profile.o $(LIBS)

vehprof: veh_wrprof.o vehicle_profile.o
	$(LINK) $(LINKFLAGS) -o $@ veh_wrprof.o vehicle_profile.o $(LIBS)

long_wrprof.o:	wrprof.c long_out_profile.h profile.h
	$(CC) $(CCFLAGS) -DLONG_OUT_PROFILE -o $@ wrprof.c 

veh_wrprof.o:	wrprof.c vehicle_profile.h profile.h
	$(CC) $(CCFLAGS) -DVEHICLE_PROFILE -o $@ wrprof.c 

long_out_profile.o:	profile.c long_out_profile.c profile.h long_out_profile.h
	$(CC) $(CCFLAGS) -DLONG_OUT_PROFILE -o $@ profile.c 

vehicle_profile.o:	profile.c vehicle_profile.c profile.h vehicle_profile.h
	$(CC) $(CCFLAGS) -DVEHICLE_PROFILE -o $@ profile.c 

install: all
	cp -f $(LONG_EXECS) /home/truckcontrol/long_ctl/src/test

clean:
	/bin/rm -f *.o  *.err $(LONG_EXECS)

distclean: clean
	cd ../../test; /bin/rm -f $(LONG_EXECS)
