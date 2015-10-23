# Makefile
#
# Copyright (c) 2006   Regents of the University of California
#
include $(CAPATH_MK_DEFS)

HDRS = gpio.h
GPIO_LIB = $(OBJDIR)/libgpio.a
GPIO_OBJS = $(OBJDIR)/gpio_lib.o
DSCUD_DIR = /usr/local/dscud5
PATH_CFLAGS += -I$(DSCUD_DIR)
PATH_LDFLAGS += -L$(DSCUD_DIR)
PATH_LIBS += -ldscud5 -lgpio

# Uncomment for debugging using gdb
#PATH_CFLAGS += -g

EXEC =  $(OBJDIR)/gpio 
	
all: 	$(OBJDIR) $(GPIO_LIB) $(EXEC)	

$(GPIO_LIB): $(GPIO_OBJS)
	$(PATH_AR) $(PATH_AR_FLAGS) $@ $(GPIO_OBJS)
	cp $(GPIO_LIB) $(SENS_LIB_DIR)/$(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/gpio: $(OBJDIR)/gpio.o

clean: 
	\rm -f *.o $(OBJDIR)/*.o $(EXEC)

install:
	\cp $(EXEC) ../test

# DO NOT DELETE
