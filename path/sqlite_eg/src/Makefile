
# Makefile - sqlite example programs used
#
# Copyright (c) 2008   Regents of the University of California
#
# For compile outside Savari SOBOS SDK, make sure that environment
# variable LD_LIBRARY_PATH has sqlite3 library directory (typically
# /usr/local/lib

include $(CAPATH_MK_DEFS)
INSTALL_DIR = ../bin

PATH_LIBS += -lpath_gps -ldb_clts -llocals -lsqlite3 -lm 

EXEC = $(OBJDIR)/coswrcvmap \
	$(OBJDIR)/coswrcvwarn \
	$(OBJDIR)/gemssnd \
	$(OBJDIR)/pt2json \
	$(OBJDIR)/ptreplay \
	$(OBJDIR)/ptrcv \
	$(OBJDIR)/ptsnd \
	$(OBJDIR)/gpsinsert \
	$(OBJDIR)/loop_test \
	$(OBJDIR)/ptrel \
	$(OBJDIR)/rcvdbg_somak \
	$(OBJDIR)/scrcv \
	$(OBJDIR)/tsparcv

all: $(OBJDIR) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/coswrcvmap: $(OBJDIR)/coswrcvmap.o
$(OBJDIR)/coswrcvwarn: $(OBJDIR)/coswrcvwarn.o
$(OBJDIR)/gemssnd: $(OBJDIR)/gemssnd.o
$(OBJDIR)/gpsinsert: $(OBJDIR)/gpsinsert.o
$(OBJDIR)/loop_test: $(OBJDIR)/loop_test.o
$(OBJDIR)/ptsnd: $(OBJDIR)/ptsnd.o
$(OBJDIR)/ptrcv: $(OBJDIR)/ptrcv.o
$(OBJDIR)/ptrel: $(OBJDIR)/ptrel.o
$(OBJDIR)/ptreplay: $(OBJDIR)/ptreplay.o
$(OBJDIR)/pt2json: $(OBJDIR)/pt2json.o
$(OBJDIR)/rcvdbg_somak: $(OBJDIR)/rcvdbg_somak.o
$(OBJDIR)/scrcv: $(OBJDIR)/scrcv.o
$(OBJDIR)/tsparcv: $(OBJDIR)/tsparcv.o

install:
	cp $(EXEC) $(INSTALL_DIR) 

clean:
	rm -f $(OBJDIR)/*.o *.err $(EXEC)

