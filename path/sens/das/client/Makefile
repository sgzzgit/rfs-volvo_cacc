#\file Makefile for QNX6 DAS (Data Acquisition System) resource managers

include $(CAPATH_MK_DEFS)

PATH_LDFLAGS += -L$(OBJDIR) 

EXECS = $(OBJDIR)/rddmm32

all: $(OBJDIR) $(MAN_LIB) $(EXECS) 

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	\rm -f $(EXECS) $(OBJDIR)/*.o

install:
	cp $(EXECS) ../test
