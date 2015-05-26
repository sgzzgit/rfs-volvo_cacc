                 
                     Lib development record  from  03_24_11

(1) long_lib_091010_3trk_6m: is the Lib for 3 truck test at 6[m] on 09_15_10

(2) From long_lib_091010_3trk_6m; vary_ref.c has been added to Makefile on 03_24_11;

(3) in maneuver.c: tran_ref() is replaced with vary_ref() since the former can only do fixed Max_Spd; the latter can do any speed transition on-the-fly (03_25_11);

(4) Road grade has been incoprated with rd_grade(); maneuver() has been changed accordingly; 