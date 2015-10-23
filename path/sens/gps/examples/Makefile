
# Makefile - PATH GPS sample programs
#
# Copyright (c) 2008   Regents of the University of California
#

HDRS= $(SENS_INC_DIR)/path_gps_lib.h

include $(CAPATH_MK_DEFS)

PATH_CFLAGS += -I$(ATSC_INC_DIR) -I$(CICAS_SRC_DIR)
#PATH_CFLAGS += -g
PATH_LIBS = -lpath_gps $(PATH_CORE_LIBS) 

EXEC = $(OBJDIR)/test_convert \
	$(OBJDIR)/tsprcv $(OBJDIR)/gpslatlong $(OBJDIR)/gpsdistance \
	 $(OBJDIR)/bcnrcv $(OBJDIR)/gpssetdate $(OBJDIR)/gpssnd \
	 $(OBJDIR)/gpsrcv $(OBJDIR)/tcp_client $(OBJDIR)/tcp_echo \
	 $(OBJDIR)/gpsdb $(OBJDIR)/gpsget $(OBJDIR)/macget \
	 $(OBJDIR)/gps2xy $(OBJDIR)/gpsdemo $(OBJDIR)/gpspoint \
	 $(OBJDIR)/rdgpsdb $(OBJDIR)/garmenable

all: $(OBJDIR) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/garmenable: $(OBJDIR)/garmenable.o
$(OBJDIR)/gpsdb: $(OBJDIR)/gpsdb.o
$(OBJDIR)/gpsdistance: $(OBJDIR)/gpsdistance.o
$(OBJDIR)/gpslatlong: $(OBJDIR)/gpslatlong.o
$(OBJDIR)/gpssetdate: $(OBJDIR)/gpssetdate.o
$(OBJDIR)/gpssnd: $(OBJDIR)/gpssnd.o
$(OBJDIR)/gpsget: $(OBJDIR)/gpsget.o
$(OBJDIR)/gpsrcv: $(OBJDIR)/gpsrcv.o
$(OBJDIR)/gps2xy: $(OBJDIR)/gps2xy.o
$(OBJDIR)/gpsdemo: $(OBJDIR)/gpsdemo.o
$(OBJDIR)/gpspoint: $(OBJDIR)/gpspoint.o
$(OBJDIR)/bcnrcv: $(OBJDIR)/bcnrcv.o
$(OBJDIR)/macget: $(OBJDIR)/macget.o
$(OBJDIR)/rdgpsdb: $(OBJDIR)/rdgpsdb.o
$(OBJDIR)/test_convert: $(OBJDIR)/test_convert.o
$(OBJDIR)/tsprcv: $(OBJDIR)/tsprcv.o
$(OBJDIR)/tcp_client: $(OBJDIR)/tcp_client.o
$(OBJDIR)/tcp_echo: $(OBJDIR)/tcp_echo.o

clean:
	rm -f $(OBJDIR)/*.o *.err $(EXEC)

