# Makefile for PATH utilities to use the Tilcon package

include $(CAPATH_MK_DEFS)

PATH_CFLAGS += -I$(TILCON_INC_DIR) 
PATH_CFLAGS += -g 

TILC_LIB = $(OBJDIR)/libtilc.a
TILC_OBJS = $(OBJDIR)/tilcon_utils.o

HDRS = widget.h tilcon_utils.h 

all:  	$(OBJDIR) $(TILC_LIB) clients	

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TILC_LIB):	$(HDRS) $(TILC_OBJS) 
	$(PATH_AR) $(PATH_AR_FLAGS) $@ $(TILC_OBJS) 

clients: $(wildcard client/src/*.[ch])
	(cd client/src; make)

clean:
	\rm -f $(OBJDIR)/*.o *.err $(TILC_LIB)
	(cd client/src; make clean)
