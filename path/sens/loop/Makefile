#
# Makefile
#


include $(CAPATH_MK_DEFS)

GPS_DIR = $(CAPATH_DIR)/sens/gps/src
LOCAL = ../..
CC = gcc
CPP = g++
DFLAGS = -g
#OFLAGS = -O2
FLAGS = #-DCAPTURE_2255 #-DCAPTURE_PICASSO
INCLUDE = -I$(LOCAL)/include 
INCLUDE += -I../Common
INCLUDE += -I$(LOCAL_DIR) -I$(DB_DIR) -I$(GPS_DIR)
#INCLUDE += -I/home/usr/include
LDFLAGS = -L$(LOCAL)/lib -L../Common
LDFLAGS += -L$(LOCAL_DIR) -L$(DB_DIR)

#LDLIBS = -ltracker -lcvvideoplayer -lcvzulib -lzumllib -lzulib -lhighgui -lcv -lcxcore
LDLIBS += $(PATH_LIBS)
#LDLIBS += -ljpeg -lzlib
#LDLIBS += -lxine -lavformat -lavcodec -lavutil
LDLIBS += -lm
#QNXLIBS = -lsocket -lph -lphrender #-lpcss

#HDRS = clt_vars.h ids_io.h

DEPEND = .depend

C_OBJS = loop2db.o
CC_OBJS = 
CPP_OBJS =
OBJS = $(C_OBJS) $(CC_OBJS) $(CPP_OBJS)

TARGET = loop2db 

all: $(TARGET) 

$(TARGET): $(OBJS)
	$(CPP) -o $(TARGET) $(DFLAGS) loop2db.o $(LDFLAGS) $(LDLIBS)

dep:	depend

depend:
	rm $(DEPEND)
	@$(MAKE) $(DEPEND)

$(DEPEND):
	@-rm -f $(DEPEND)
	@echo "Generating dependencies... "
	@echo > $(DEPEND)
	@if ! test -z $(firstword $(C_OBJS)); then \
		$(CC) $(FLAGS) $(INCLUDE) -MM \
		   `echo $(C_OBJS) | sed -e "s/\.o/\.c/g"` >> $(DEPEND); \
	 fi
	@if ! test -z $(firstword $(CC_OBJS)); then \
		$(CC) $(FLAGS) $(INCLUDE) -MM \
		   `echo $(CC_OBJS) | sed -e "s/\.o/\.cc/g"` >> $(DEPEND); \
	 fi
	@if ! test -z $(firstword $(CPP_OBJS)); then \
		$(CPP) $(FLAGS) $(INCLUDE) -MM \
		   `echo $(CPP_OBJS) | sed -e "s/\.o/\.cpp/g"` >> $(DEPEND); \
	 fi

-include $(DEPEND)

%.o: %.c $(DEPEND)
	$(CC) $(OFLAGS) $(FLAGS) $(DFLAGS) $(INCLUDE) -o $@ -c $<

%.o: %.cc $(DEPEND)
	$(CC) $(OFLAGS) $(FLAGS) $(DFLAGS) $(INCLUDE) -o $@ -c $<

%.o: %.cpp $(DEPEND)
	$(CPP) $(OFLAGS) $(FLAGS) $(DFLAGS) $(INCLUDE) -o $@ -c $<

.PHONY:	clean
clean:
	rm -rf $(DEPEND) $(OBJS) $(TARGET)

.SUFFIXES: .c .cc .cpp .o 

