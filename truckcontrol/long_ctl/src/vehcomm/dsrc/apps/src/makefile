# makefile
include $(CAPATH_MK_DEFS)

ASNINDIR 					:= ../../asn1/include
ASNLIBDIR 				:= ../../asn1/release
ASNJ2375INDIR			:= ../../asn1j2735/include
ASNJ2375LIBDIR 		:= ../../asn1j2735/release
MMITSSCOMMINDIR		:= ../../comm/include
MMITSSCOMMLIBDIR	:= ../../comm/release
INCDIR  					:= ../include
RELDIR  					:= ../release
OBJDIR  					:= ./qnx
ASNLIB 						:= -L$(ASNLIBDIR) -lasn
ASNJ2735LIB 			:= -L$(ASNJ2375LIBDIR) -ldsrc
MMITSSCOMMLIB			:= -L$(MMITSSCOMMLIBDIR) -lmmitsscomm 

COMP := -Wall -W -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -O3 -fPIC -fno-strict-aliasing 
# -DNDEBUG to disable -DNDEBUG assert

ALLINC := -I/usr/include -I$(INCDIR) -I$(ASNINDIR) -I$(ASNJ2375INDIR) -I$(MMITSSCOMMINDIR)
COMPFLAGS := $(COMP) $(ALLINC)

C++ := g++

EXEC := $(OBJDIR)/testMap\
	$(OBJDIR)/testMapDecode\
	$(OBJDIR)/rsuAware
	
all: $(OBJDIR) $(RELDIR) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(RELDIR):
	mkdir -p $(RELDIR)	

$(OBJDIR)/testMap: testMap.cpp 
	$(C++)  $(COMPFLAGS) testMap.cpp -o $(OBJDIR)/testMap $(ASNJ2735LIB)
	
$(OBJDIR)/testMapDecode: testMapDecode.cpp 
	$(C++)  $(COMPFLAGS) testMapDecode.cpp -o $(OBJDIR)/testMapDecode $(ASNJ2735LIB) $(ASNLIB)
	
$(OBJDIR)/rsuAware: rsuAware.cpp $(INCDIR)/rsuAware.h 
	$(C++)  $(COMPFLAGS) $(PATH_LDFLAGS) rsuAware.cpp -o $(OBJDIR)/rsuAware $(MMITSSCOMMLIB) $(ASNJ2735LIB) $(ASNLIB) $(PATH_LIBS)
	
clean:
	rm -f $(EXEC)

install:
	cp $(EXEC) $(RELDIR)


