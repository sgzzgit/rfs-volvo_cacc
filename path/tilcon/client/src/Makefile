# Makefile
#
# Copyright (c) 2008   Regents of the University of California
#
include $(CAPATH_MK_DEFS)

PATH_CFLAGS += $(TILCON_CFLAGS)
PATH_LDFLAGS += $(TILCON_LDFLAGS) 
PATH_LIBS += $(TILCON_LIBS)

EXEC = $(OBJDIR)/sample $(OBJDIR)/obm_test $(OBJDIR)/obm_chg_vals $(OBJDIR)/chg_vals

.o:
	$(PATH_LINKPP) $(LDFLAGS) $(PATH_LDFLAGS) -o $@ $@.o $(PATH_LIBS) 

all: $(OBJDIR) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/obm_test:	$(OBJDIR)/obm_test.o 
$(OBJDIR)/sample:	$(OBJDIR)/sample.o 
$(OBJDIR)/chg_vals:	$(OBJDIR)/chg_vals.o 
$(OBJDIR)/obm_chg_vals:	$(OBJDIR)/obm_chg_vals.o 

clean:
	rm -f $(OBJDIR)/*.o *.err *.a $(EXEC)

install:
	cp $(EXEC) ../test



