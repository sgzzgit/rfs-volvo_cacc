#\file Makefile for QNX6 DAS (Data Acquisition System) resource managers

include $(CAPATH_MK_DEFS)

MANOBJ = $(OBJDIR)/das_default.o $(OBJDIR)/das_init.o \
	 $(OBJDIR)/das_man.o $(OBJDIR)/das_util.o $(OBJDIR)/io_func.o \
	 $(OBJDIR)/am9513.o 

MANHDRS = $(LOCAL_DIR)/das_clt.h das_man.h  

MAN_LIB = $(OBJDIR)/libdasman.a

PATH_LIBS = -ldasman $(PATH_CORE_LIBS)

PATH_LDFLAGS += -L$(OBJDIR) 

EXECS = $(OBJDIR)/null_das $(OBJDIR)/garnet $(OBJDIR)/dmm32\
	$(OBJDIR)/qmm10xt $(OBJDIR)/rdiop $(OBJDIR)/wriop\
	$(OBJDIR)/regs_dmm32 $(OBJDIR)/io_lnx

all: $(OBJDIR) $(MAN_LIB) $(EXECS) 

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(MAN_LIB): $(MANHDRS) $(MANOBJ)
	 $(PATH_AR) $(PATH_AR_FLAGS)  $@ $(MANOBJ)

clean:
	\rm -f $(MANOBJ) $(EXECS) $(OBJDIR)/*.o $(OBJDIR)/*.a

install:
	chown root $(EXECS)
	chmod 4775 $(EXECS)
