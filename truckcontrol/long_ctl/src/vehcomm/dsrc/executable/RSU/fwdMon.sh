#!/bin/sh
RETCODE=1
PROGPATH="/usr/local/bin/path"
CONFIGPATH="/nojournal/bin/path/config"
PROGNAME="wmefwd"
GREPARGS="rsuWmefwd.config"
PROGARGS="-c $CONFIGPATH/$GREPARGS -f"

/usr/bin/killall  -TERM "$PROGNAME"
sleep 1s

if [ -e "$PROGPATH"/"$PROGNAME" ];then
  while [ 1 ];do
    ps -ef | grep "$PROGNAME" | grep -v grep > /dev/null
    RETCODE=$?  
    if [ $RETCODE -eq 1 ]; then
      echo "starting $PROGNAME"
      exec "$PROGPATH/$PROGNAME" $PROGARGS &
      sleep 2s
    fi
    sleep 60s
  done
fi  