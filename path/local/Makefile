# \file
# Makefile for local libraries used by PATH shared code
#
# CAPATH_MK_DEFS must be set in the environment to correct file of
# definitions for the compile environment (lnx, qnx or sobos) (check
# in /home/path/build for an appropriate makefile)
#
# Copyright (c) 2008   Regents of the University of California
#

include $(CAPATH_MK_DEFS)

PATH_CFLAGS += -g

LOCAL_OBJS = \
	$(OBJDIR)/data_log.o \
	$(OBJDIR)/matrix.o \
	$(OBJDIR)/pack.o \
	$(OBJDIR)/st.o \
	$(OBJDIR)/sys_arg.o \
	$(OBJDIR)/sys_arma.o \
	$(OBJDIR)/sys_buff.o \
	$(OBJDIR)/sys_das.o \
	$(OBJDIR)/sys_fifo.o \
	$(OBJDIR)/sys_ini.o \
	$(OBJDIR)/sys_lib.o \
	$(OBJDIR)/sys_list.o \
	$(OBJDIR)/sys_rt.o \
	$(OBJDIR)/sys_str.o \
	$(OBJDIR)/sys_tok.o \
	$(OBJDIR)/sys_time.o \
	$(OBJDIR)/sys_trk.o \
	$(OBJDIR)/timing.o \
	$(OBJDIR)/timestamp.o \
	$(OBJDIR)/tcp_utils.o \
	$(OBJDIR)/util.o \
	$(OBJDIR)/udp_utils.o \
	$(OBJDIR)/xytransform.o 

LOCAL_HDRS = data_log.h \
	local.h \
	matrix.h \
	pack.h \
	st.h \
	sys_arma.h \
	sys_btr.h \
	sys_buff.h \
	sys_das.h \
	sys_fifo.h \
	sys_ini.h \
	sys_lib.h \
	sys_list.h \
	sys_mem.h \
	sys_rt.h \
	sys_str.h \
	sys_time.h \
	sys_tok.h \
	sys_trk.h \
	timing.h \
	timestamp.h\
	util.h \
	udp_utils.h 

OUR_LIBRARIES = $(OBJDIR)/liblocals.a

all:	sys_os.h $(OBJDIR) $(OUR_LIBRARIES)

sys_os.h:
	ln -s $(SYS_OS_HEADER_FILE) sys_os.h

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/liblocals.a: $(LOCAL_OBJS) $(LOCAL_HDRS)
	$(PATH_AR) $(PATH_AR_FLAGS) $@ $(LOCAL_OBJS)

clean:
	\rm -rf $(OBJDIR)/*.o *.err $(OUR_LIBRARIES)
