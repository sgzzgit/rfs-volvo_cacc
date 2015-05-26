VAAPID=`ps -aef | grep vaa_slave | grep -v grep | awk '{print $2}'`
kill -9 $VAAPID
sleep 1
slay -f -Q can_rx 
slay -f -Q can_tx 
slay -f -Q nc 
slay -f -Q can_dual 
