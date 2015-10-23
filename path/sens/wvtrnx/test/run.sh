#!/bin/sh
# running the wavetronix

/home/path/db/lnx/db_slv & 
sleep 1
/home/path/sens/wvtrnx/test/wvtrnx_create &
sleep 1
/home/path/sens/wvtrnx/test/wvtrnx -d -v -s /dev/ttyS0 1>wvtrnx.log 2>wvtrnx.err &
sleep 1
/home/path/sens/wvtrnx/test/wvtrnx_wrfile -i 100 -v 1>wrfile.log &
