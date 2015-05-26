This directory contains the files necessary to gnuplot the data contained in
the engineering e-file for truckcontrol. They are:

trk.in - contains a list of the desired columns from the data file, and a 
	title for each plot.  The format is:
	Plot_title	data_file_column_number 

	The list is current as of 04/06/2010.  If 
	/home/truckcontrol/long_ctl/src/avcs/trk.wr.c changes, trk.in
	should be changed accordingly.

trk.gpl - contains the actual commands sent to gnuplot, for plotting trk.in.
	It assumes the data file is named "test.dat", so the user must copy 
	the data file to "test.dat". WARNING - if a file "test.dat" already
	exists, it will be overwritten with the data file.

	trk.gpl is created by issuing the command:
	/home/path/plot/create_pl <trk.in>trk.gpl

	If the binary create_pl does not exist, it can be compiled from
	/home/path/plot/create_pl.c (which _should_ exist) with the command:
	make /home/path/plot/create_pl
	
drop_check.sh - script for checking  trk_wr.dat-type files for dropped 
	data. This is done by converting the timestamp in the first column 
	to seconds-since-midnight with fixtime.sh, and then subtracting 
	successive times to see whether any differences are greater than 
	75 msec (should be 50). If so, the line number is returned.

drop_check_xyl.sh - # Check Xiao-Yun's test.dat-type files for dropped data. 
	This is done by subtracting successive timestamps to see whether any 
	differences are greater than 150 msec (should be 100). If so, the 
	line number is returned.

long_trk.in - contains a list of the desired columns from Xiao-Yun's data file,
	and a title for each plot.  The format is:
	Plot_title	data_file_column_number 

	The list is current as of 04/08/2010.  If 
	/home/truckcontrol/long_ctl/src/xyl/long_trk.c changes, long_trk.in
	should be changed accordingly.

long_trk.gpl - contains the actual commands sent to gnuplot, for plotting 
	long_trk.in. It assumes the data file is named "test.dat", so the 
	user must copy the data file to "test.dat". WARNING - if a file 
	"test.dat" already exists, it will be overwritten with the data file.

	long_trk.gpl is created by issuing the command:
	/home/path/plot/create_pl <long_trk.in>long_trk.gpl

	If the binary create_pl does not exist, it can be compiled from
	/home/path/plot/create_pl.c (which _should_ exist) with the command:
	make /home/path/plot/create_pl

go_xyl - script for plotting the data file of interest.  go_xyl copies the data
	file (given as the argument to go_xyl) to "test.dat", and executes 
	"gnuplot long_trk.gpl".
