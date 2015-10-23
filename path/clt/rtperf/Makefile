# Makefile for clock set and monitor programs
HDRS = clock_set.h
include $(CAPATH_MK_DEFS)

EXECS =  $(OBJDIR)/rt_snd $(OBJDIR)/rt_rcv $(OBJDIR)/rt_mtr \
	 $(OBJDIR)/clock_set $(OBJDIR)/clock_snd_skew

all:  	$(OBJDIR) $(EXECS)	

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/rt_snd:	$(OBJDIR)/rt_snd.o 

$(OBJDIR)/rt_rcv:	$(OBJDIR)/rt_rcv.o 

$(OBJDIR)/rt_mtr:	$(OBJDIR)/rt_mtr.o 

$(OBJDIR)/clock_snd_skew:	$(OBJDIR)/clock_snd_skew.o 

$(OBJDIR)/clock_set:	$(OBJDIR)/clock_set.o 

clean:
	rm -f $(OBJDIR)/*.o *.err $(EXECS)
