# makefile
ASNINDIR := ../../asn1/include
ASNLIBDIR := ../../asn1/release
INCDIR  := ../include
RELDIR  := ../release
OBJDIR  := ../obj

ASNLIB := -L$(ASNLIBDIR) -lasn

COMP := -Wall -W -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -O3 -fPIC -fno-strict-aliasing 
# -DNDEBUG to disable -DNDEBUG assert

ALLINC := -I/usr/include -I$(INCDIR) -I$(ASNINDIR) 
COMPFLAGS := $(COMP) $(ALLINC)

C++ := g++
AR := ar
ARFLAGS := -rv

OBJS := $(OBJDIR)/MapDataStruct.o\
	$(OBJDIR)/AsnJ2735Lib.o\
	$(OBJDIR)/geoUtils.o\
	$(OBJDIR)/wmeUtils.o
	
DSRCLIB := $(OBJDIR)/libdsrc.a
	
all: $(OBJDIR) $(RELDIR) $(OBJS) $(DSRCLIB)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(RELDIR):
	mkdir -p $(RELDIR)	

$(OBJDIR)/AsnJ2735Lib.o: AsnJ2735Lib.cpp  $(INCDIR)/AsnJ2735Lib.h $(INCDIR)/MapDataStruct.h $(INCDIR)/dsrcConsts.h $(INCDIR)/geoUtils.h
	$(C++)  $(COMPFLAGS) -c AsnJ2735Lib.cpp -o $(OBJDIR)/AsnJ2735Lib.o
	
$(OBJDIR)/MapDataStruct.o: MapDataStruct.cpp  $(INCDIR)/MapDataStruct.h $(INCDIR)/geoUtils.h $(INCDIR)/dsrcConsts.h
	$(C++)  $(COMPFLAGS) -c MapDataStruct.cpp -o $(OBJDIR)/MapDataStruct.o 
	
$(OBJDIR)/geoUtils.o: geoUtils.cpp  $(INCDIR)/geoUtils.h $(INCDIR)/dsrcConsts.h
	$(C++)  $(COMPFLAGS) -c geoUtils.cpp -o $(OBJDIR)/geoUtils.o

$(OBJDIR)/wmeUtils.o: wmeUtils.cpp  $(INCDIR)/wmeUtils.h
	$(C++)  $(COMPFLAGS) -c wmeUtils.cpp -o $(OBJDIR)/wmeUtils.o
	
$(DSRCLIB): $(OBJS)
	$(AR) $(ARFLAGS) -o $(DSRCLIB) $(OBJS)	

clean:
	rm -f $(OBJS) $(DSRCLIB)

install:
	cp $(DSRCLIB) $(RELDIR)


