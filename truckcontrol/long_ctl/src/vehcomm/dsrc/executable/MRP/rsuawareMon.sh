#!/bin/sh
RETCODE=1
PROGPATH="/home/rfs-MMITSS/bin"
CONFIGPATH="/home/rfs-MMITSS/config"
PROGNAME="rsuAware"
PROGARGS="-s $CONFIGPATH/rsuSocket.config -n `hostname` -f"

/usr/bin/killall  -TERM "$PROGNAME"
sleep 1s

if [ -e "$PROGPATH"/"$PROGNAME" ];then
  while [ 1 ];do
    ps -ef | grep "$PROGNAME" | grep -v grep > /dev/null
    RETCODE=$?  
    if [ $RETCODE -eq 1 ]; then
      /usr/bin/screen -d -m "$PROGPATH/$PROGNAME" $PROGARGS
      sleep 2s
    fi
    sleep 60s
  done
fi  