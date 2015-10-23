# Makefile for California PATH sensor libraries and example prorams
#
# Copyright (c) 2008   Regents of the University of California
#

include $(CAPATH_MK_DEFS)

SENS_LIBRARIES =\
	gps/src/$(OBJDIR)/libpath_gps.a \
	radar/$(OBJDIR)/libevt300.a 
#	sms/src/$(OBJDIR)/libsms.a 


all: includes lib/$(OBJDIR) $(SENS_LIBRARIES) $(SENS_DRIVERS) 
# Uncomment the following for QNX6, they break the Linux build.
	(cd lidar/denso; make)
	(cd das/driver; make)
	(cd das/client; make)

includes:
	./make_includes

lib/$(OBJDIR):
	(cd lib; make)

gps/src/$(OBJDIR)/libpath_gps.a: $(wildcard gps/src/*.[ch])
	(cd gps; make);

lidar/ibeo_can/$(OBJDIR)/libibeo_utils.a: $(wildcard lidar/ibeo_can/*.[ch])
	(cd lidar; make)

radar/$(OBJDIR)/libevt300.a: $(wildcard radar/*.[ch])
	(cd radar; make)

sms/src/$(OBJDIR)/libsms.a: $(wildcard sms/src/*.[ch])
#	(cd sms; make)

clean:
	(cd gps; make clean)
	(cd radar; make clean)
	(cd lidar/denso; make clean)
	(cd lidar/mdl; make clean)
	(cd sms; make clean)
	(cd das/driver; make clean)
	(cd das/client; make clean)
	(cd lib; make clean)

install: all
	make install -C can
