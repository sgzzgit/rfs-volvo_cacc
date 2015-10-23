# Makefile for mtcp sample programs 
#
# make test: makes and runs unit tests
#
# make docs: make docs using doxygen, then browse at docs/html/index.html
#
# make tags: make tags file file using ctags
#
# make clean
#

include $(CAPATH_MK_DEFS)

PATH_CFLAGS += -I.. 
# Comment this out for production code:
PATH_CFLAGS += -DDEBUG=1
PATH_LIBS += -lmtcp
PATH_LDFLAGS += -L../$(OBJDIR)

ifeq ($(shell uname),QNX)
  PATH_CFLAGS += -DQNX=1
  PATH_LIBS += -lsocket
endif

OBJS = $(OBJDIR)/mtcp.o $(OBJDIR)/sockhop-client.o

EXECS = $(OBJDIR)/sub $(OBJDIR)/pub $(OBJDIR)/read

all: $(OBJDIR) $(EXECS)  

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/sub:	$(OBJDIR)/sub.o 
$(OBJDIR)/pub:	$(OBJDIR)/pub.o 
$(OBJDIR)/read:	$(OBJDIR)/read.o 

clean:
	\rm -f $(OBJDIR)/*.o */*.o *.err $(EXECS)
