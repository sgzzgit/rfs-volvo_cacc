# makefile
ASNJ2375INDIR		:= ../../asn1j2735/include
INCDIR  				:= ../include
RELDIR  				:= ../release
OBJDIR  				:= ../obj

COMP := -Wall -W -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -O3 -fPIC -fno-strict-aliasing 
# -DNDEBUG to disable -DNDEBUG assert

ALLINC := -I/usr/include -I$(INCDIR) -I$(ASNJ2375INDIR) 
COMPFLAGS := $(COMP) $(ALLINC)

C++ := g++
AR := ar
ARFLAGS := -rv

OBJS := $(OBJDIR)/obuConfig.o\
	$(OBJDIR)/rsuConfig.o\
	$(OBJDIR)/emConfig.o\
	$(OBJDIR)/comm.o\
	$(OBJDIR)/socketUtils.o\
	$(OBJDIR)/timeUtils.o
	
MMITSSCOMMLIB := $(OBJDIR)/libmmitsscomm.a
	
all: $(OBJDIR) $(RELDIR) $(OBJS) $(MMITSSCOMMLIB)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(RELDIR):
	mkdir -p $(RELDIR)	

$(OBJDIR)/obuConfig.o: obuConfig.cpp  $(INCDIR)/obuConfig.h
	$(C++)  $(COMPFLAGS) -c obuConfig.cpp -o $(OBJDIR)/obuConfig.o
	
$(OBJDIR)/emConfig.o: emConfig.cpp  $(INCDIR)/emConfig.h
	$(C++)  $(COMPFLAGS) -c emConfig.cpp -o $(OBJDIR)/emConfig.o
	
$(OBJDIR)/rsuConfig.o: rsuConfig.cpp  $(INCDIR)/rsuConfig.h
	$(C++)  $(COMPFLAGS) -c rsuConfig.cpp -o $(OBJDIR)/rsuConfig.o
	
$(OBJDIR)/comm.o: comm.cpp  $(INCDIR)/comm.h
	$(C++)  $(COMPFLAGS) -c comm.cpp -o $(OBJDIR)/comm.o
	
$(OBJDIR)/socketUtils.o: socketUtils.cpp  $(INCDIR)/socketUtils.h
	$(C++)  $(COMPFLAGS) -c socketUtils.cpp -o $(OBJDIR)/socketUtils.o
  
$(OBJDIR)/timeUtils.o: timeUtils.cpp  $(INCDIR)/timeUtils.h
	$(C++)  $(COMPFLAGS) -c timeUtils.cpp -o $(OBJDIR)/timeUtils.o
    
$(MMITSSCOMMLIB): $(OBJS)
	$(AR) $(ARFLAGS) -o $(MMITSSCOMMLIB) $(OBJS)	

clean:
	rm -f $(OBJS) $(MMITSSCOMMLIB)

install:
	cp $(MMITSSCOMMLIB) $(RELDIR)


