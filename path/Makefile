# Makefile for California PATH shared code
#
# Copyright (c) 2008   Regents of the University of California
#
# CAPATH_MK_DEFS must be set in the environment to correct file of
# definitions forthe compile environment (lnx, bsd, qnx or sobos)
# Check in ../build for appropriate file.

include $(CAPATH_MK_DEFS)

# mtcp and Tilcon libraries are not built by default
PATH_LIBRARIES = $(LOCAL_DIR)/$(OBJDIR)/liblocals.a\
	$(DB_DIR)/$(OBJDIR)/libdb_clts.a
#	$(TILC_DIR)/$(OBJDIR)/libtilc.a
#	mtcp/$(OBJDIR)/libmtcp.a 

# not all libraries in sens and sqlite_eg trees may be built for all systems 
all: setup $(PATH_LIBRARIES) 
	(cd $(CAPATH_DIR)/clt; make)
	(cd $(CAPATH_DIR)/sens; make)
#	(cd $(CAPATH_DIR)/sqlite_eg; make)

$(LOCAL_DIR)/$(OBJDIR)/liblocals.a: $(wildcard local/*.[ch])
	(cd $(LOCAL_DIR); make)

$(DB_DIR)/$(OBJDIR)/libdb_clts.a: $(wildcard db/*.[ch])
	(cd $(DB_DIR); make)

mtcp/$(OBJDIR)/libmtcp.a: $(wildcard mtcp/*.[ch])
#	(cd $(CAPATH_DIR)/mtcp; make)

$(TILC_DIR)/$(OBJDIR)/libtilc.a: $(wildcard tilcon/*.[ch])
#	(cd $(TILC_DIR); make)

clean:
	(cd $(LOCAL_DIR); make clean)
	(cd $(DB_DIR); make clean)
	(cd $(CAPATH_DIR)/clt; make clean)
	(cd $(CAPATH_DIR)/sens; make clean)
#	(cd $(CAPATH_DIR)/mtcp; make clean)
#	(cd $(TILC_DIR); make clean)
	(rm -f setup.*)

# This target is not useful for the Savari build, which has a more
# complicated distribution method using ipkg that is specified
# in the top-level Makefile.sobu.
install:
	cp $(PATH_LIBRARIES) $(RELEASE_DIR)
#	make install -C sens

# This rule makes sure that all files are set up correctly for
# the system specifed by $(CAPATH_MK_DEFS), in case you are changing
# back and forth between native and cross-compiles in the same
# checkout tree. Libraries and executables will be put in subdirectories
# as specified by OBJDIR. This definition is also used to specify
# a system specific target.

setup: $(OBJDIR)
	(cd $(LOCAL_DIR); ln -f -s $(SYS_OS_HEADER_FILE) sys_os.h)

# The following targets can be used to add any additional configuration
# required to set up the compile for different target architectures 

cyg:
	(echo 'Windows Cygwin compile' > setup.$(OBJDIR))

sobu:
	(cp $(CAPATH_DIR)/build/Makefile.$(OBJDIR) $(DISTRIB_DIR)/Makefile)
	(echo 'Cross compile for Savari on-board unit' > setup.$(OBJDIR))

lnx:
	(cp $(CAPATH_DIR)/build/Makefile.$(OBJDIR) $(DISTRIB_DIR)/Makefile)
	(echo 'Native Linux compile' > setup.$(OBJDIR))

qnx:
#	(cp $(CAPATH_DIR)/build/Makefile.$(OBJDIR) $(DISTRIB_DIR)/Makefile)
	(echo 'Native QNX6 compile' > setup.$(OBJDIR))

bsd:
	if ! [ -e /usr/local/include/malloc.h ]; then ln -s /usr/local/include/sys/malloc.h /usr/local/include/malloc.h; fi
	mkdir -p $(SENS_LIB_DIR)/$(OBJDIR)
	(cp $(CAPATH_DIR)/build/Makefile.$(OBJDIR) $(DISTRIB_DIR)/Makefile)
	(echo 'Native FreeBSD compile' > setup.$(OBJDIR))

