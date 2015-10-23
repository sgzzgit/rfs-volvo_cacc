# makefile
CURRENTDIR				:= $(shell pwd)
ASNSRCDIR					:= $(CURRENTDIR)/asn1/src
ASNJ2375SRCDIR		:= $(CURRENTDIR)/asn1j2735/src
MMITSSCOMMSRCDIR	:= $(CURRENTDIR)/comm/src
MMITSSAPPSRCDIR		:= $(CURRENTDIR)/apps/src

#SUBDIRS := $(ASNJ2375SRCDIR) $(MMITSSCOMMSRCDIR) $(MMITSSAPPSRCDIR)
SUBDIRS := $(ASNJ2375SRCDIR) $(MMITSSCOMMSRCDIR)

all:
	for dir in $(SUBDIRS); do \
		cd $$dir && make clean && make all && make install; \
	done
