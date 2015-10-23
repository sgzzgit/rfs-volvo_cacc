# makefile

ASNJ2375INDIR			:= ../../asn1j2735/include
ASDASNJ2375OBJDIR := ../../asn1j2735/asd/obj
RSUASNJ2375OBJDIR := ../../asn1j2735/rsu/obj

MMITSSCOMMINDIR     := ../../comm/include
ASDMMITSSCOMMOBJDIR := ../../comm/asd/obj
RSUMMITSSCOMMOBJDIR := ../../comm/rsu/obj

ASDTOOLCHAIN  := ../../../Sobos-SDK-for-i686-Linux/ASD_SDK-3.1.2/toolchain
RSUTOOLCHAIN  := ../../../Sobos-SDK-for-i686-Linux/RSE_SDK-3.1.2/toolchain

INCDIR      := ../include
ASDRELDIR   := ../asd/release
ASDOBJDIR   := ../asd/obj
RSURELDIR   := ../rsu/release
RSUOBJDIR   := ../rsu/obj

ifdef ASD
RELDIR    := $(ASDRELDIR)
OBJDIR    := $(ASDOBJDIR)
ASNJ2375OBJDIR    := $(ASDASNJ2375OBJDIR)
MMITSSCOMMOBJDIR  := $(ASDMMITSSCOMMOBJDIR)
TOOLCHAIN := $(ASDTOOLCHAIN)
CFLAGS    := -g -Wall -DPRINT_LOG -DASD
else
RELDIR    := $(RSURELDIR)
OBJDIR    := $(RSUOBJDIR)
ASNJ2375OBJDIR    := $(RSUASNJ2375OBJDIR)
MMITSSCOMMOBJDIR  := $(RSUMMITSSCOMMOBJDIR)
TOOLCHAIN	:= $(RSUTOOLCHAIN)
CFLAGS		:= -g -Wall -DPRINT_LOG -DRSU
endif

#SAVARI_C++ 	:= $(TOOLCHAIN)/bin/i386-linux-c++
SAVARI_C++ 	:= $(TOOLCHAIN)/bin/i386-linux-uclibc-c++
ALLINC 			:= -I$(TOOLCHAIN)/include -I$(TOOLCHAIN)/usr/include -I $(ASNJ2375INDIR) -I$(MMITSSCOMMINDIR) -I$(INCDIR)
COMPFLAGS 	:= $(CFLAGS) $(ALLINC)

SAVARILIB		:= -L$(TOOLCHAIN)/usr/lib -lstdc++ -luClibc++ -leloop -lwme

OBJ = $(OBJDIR)/wmeconfig.o\
	$(OBJDIR)/wmefwd.o
OBJS = $(OBJDIR)/wmefwd.o\
	$(OBJDIR)/wmeconfig.o\
	$(MMITSSCOMMOBJDIR)/socketUtils.o\
	$(MMITSSCOMMOBJDIR)/timeUtils.o\
	$(ASNJ2375OBJDIR)/wmeUtils.o

EXEC := $(OBJDIR)/wmefwd
	
all: $(OBJDIR) $(RELDIR) $(OBJ) $(EXEC)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(RELDIR):
	mkdir -p $(RELDIR)	
	
$(OBJDIR)/wmeconfig.o: wmeconfig.cpp  $(INCDIR)/wmeconfig.h
	${SAVARI_C++}  $(COMPFLAGS) -c wmeconfig.cpp -o $(OBJDIR)/wmeconfig.o
	
$(OBJDIR)/wmefwd.o: wmefwd.cpp  $(INCDIR)/wmefwd.h
	${SAVARI_C++}  $(COMPFLAGS) -c wmefwd.cpp -o $(OBJDIR)/wmefwd.o
	
$(OBJDIR)/wmefwd:
	${SAVARI_C++}  $(COMPFLAGS) $(OBJS) -o $(OBJDIR)/wmefwd $(SAVARILIB)
	
clean:
	rm -f $(OBJ) $(EXEC)

install:
	cp $(EXEC) $(RELDIR)
