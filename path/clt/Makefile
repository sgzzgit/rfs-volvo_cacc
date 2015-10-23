# Makefile
#
# Copyright (c) 2006   Regents of the University of California
#
HDRS = clt_vars.h ids_io.h

include $(CAPATH_MK_DEFS)

EXEC = $(OBJDIR)/chgdii $(OBJDIR)/rcv_io $(OBJDIR)/sample_read \
	$(OBJDIR)/sample_update $(OBJDIR)/test_read $(OBJDIR)/test_update \
	$(OBJDIR)/snddbg $(OBJDIR)/rcvdbg $(OBJDIR)/fwudp \
	$(OBJDIR)/udptx $(OBJDIR)/udprx \
	$(OBJDIR)/sms2local $(OBJDIR)/timejoin $(OBJDIR)/rmcsnd

ifneq ($(CAPATH_MK_DEFS),/home/path/build/capath_qnx.mk)
	EXEC += $(OBJDIR)/rmcsnd
endif

all: $(OBJDIR) $(EXEC)
	(cd rtperf; make)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/chgdii:		$(OBJDIR)/chgdii.o 
$(OBJDIR)/rcv_io:		$(OBJDIR)/rcv_io.o 
$(OBJDIR)/fwudp:		$(OBJDIR)/fwudp.o 
$(OBJDIR)/rcvdbg:		$(OBJDIR)/rcvdbg.o 
$(OBJDIR)/rmcsnd:		$(OBJDIR)/rmcsnd.o 
$(OBJDIR)/sample_read:		$(OBJDIR)/sample_read.o
$(OBJDIR)/sample_update:	$(OBJDIR)/sample_update.o 
$(OBJDIR)/sms2local:		$(OBJDIR)/sms2local.o 
$(OBJDIR)/snddbg:		$(OBJDIR)/snddbg.o 
$(OBJDIR)/test_read:		$(OBJDIR)/test_read.o 
$(OBJDIR)/test_update:		$(OBJDIR)/test_update.o 
$(OBJDIR)/timejoin:		$(OBJDIR)/timejoin.o 
$(OBJDIR)/udptx:		$(OBJDIR)/udptx.o 
$(OBJDIR)/udprx:		$(OBJDIR)/udprx.o 

clean:
	rm -f $(OBJDIR)/*.o *.err *.a $(EXEC)
	(cd rtperf; make clean)

