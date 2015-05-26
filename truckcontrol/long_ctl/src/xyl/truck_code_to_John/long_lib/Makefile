include $(CAPATH_MK_DEFS)

# Location of files common to all projects under long_ctl/src
SRC_DIR = /home/truckcontrol/long_ctl/src
AVCS_DIR = $(SRC_DIR)/avcs
XYL_DIR = $(SRC_DIR)/xyl
LIDAR_DIR = /home/path/sens/lidar/mdl

CFLAGS = $(PATH_CFLAGS) -DPROTOTYPE
CFLAGS += -I$(AVCS_DIR) -I$(LIDAR_DIR)

TRACKLIB = libtrack.a

#
# User Defined macros for files, directories, etc.

OTHER_SRCS = acc_flt.c \
             brk_flt.c \
             cntract.c \
             control.c \
             coording.c \
             dvi.c \
             emap_525.c \
             eng_map.c \
             filt_1.c \
             filt_2.c \
             filt_4.c \
             fuel_flt.c \
             gear_tbl.c \
             gen_ref.c \
             jk_map.c \
             ldr_flt1.c\
	ldr_flt.c\
             mag_dist.c \
             maneuver.c \
             pich_flt.c \
             pm_filt.c \
             ptch_flt.c \
             rad_dist.c \
             ref_dist.c \
             ref_ini.c \
             sig_flt1.c \
             sig_flt2.c \
             sinsat.c \
             smooth.c \
             spd_flt.c \
             svg13.c \
             svg25.c \
             svg31.c \
             svg3a.c \
             svg3b.c \
             svg3c.c \
             svg3.c \
             svg5.c \
             svg73.c \
             rd_grade.c \
             usyn_flt.c \
             v_flt.c \
             mdl_flt.c \
             vary_ref.c \
             vrd_flt.c \
             vrd_flt1.c
            
OTHER_OBJS = $(OTHER_SRCS:c=o)

#
# Default rules
#

all:   $(OTHER_OBJS) $(TRACKLIB)

.c.o:
	$(CC) $(CFLAGS) -o $@ $<

$(TRACKLIB): $(OTHER_OBJS)
	$(PATH_AR) $(PATH_AR_FLAGS) $@ $(OTHER_OBJS)

install:  all
	cp -f $(TRACKLIB) $(XYL_DIR)

clean:
	/bin/rm -f *.o \
	$(TRACKLIB)

