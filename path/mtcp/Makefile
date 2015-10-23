# Makefile for mtcp and test_mtcp.
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

PATH_LIBS += -lmtcp
PATH_LDFLAGS += -L./$(OBJDIR)

# Comment this out for production code:
PATH_CFLAGS += -DDEBUG=1

ifeq ($(shell uname),QNX)
  PATH_CFLAGS += -DQNX=1
  PATH_LIBS += -lsocket
endif

SOURCES = $(wildcard *.[ch] sample/*.[ch])

MTCP_LIB = $(OBJDIR)/libmtcp.a
MTCP_OBJS = $(OBJDIR)/mtcp.o $(OBJDIR)/sockhop-client.o

TESTS = $(OBJDIR)/test-mtcp

all: $(OBJDIR) $(MTCP_LIB) $(TESTS) samples

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(MTCP_LIB): $(MTCP_OBJS)
	$(PATH_AR) $(PATH_AR_FLAGS) $@ $(MTCP_OBJS)

$(OBJDIR)/test-mtcp:	 $(OBJDIR)/test-mtcp.o

samples:
	(cd sample; make)

test: $(TESTS)
	./run-tests.sh $(TESTS)

docs: Doxyfile $(SOURCES)
	echo "making docs"
	doxygen Doxyfile

tags: $(SOURCES)
	@ echo "making ctags"
	@ ctags -R

clean:
	\rm -f $(OBJDIR)/* */*.o *.err
	(cd sample; make clean)


