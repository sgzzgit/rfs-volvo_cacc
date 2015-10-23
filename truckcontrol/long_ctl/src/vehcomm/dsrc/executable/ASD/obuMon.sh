#!/bin/sh
RETCODE=1
PROGPATH="/usr/local/bin/path"
CONFIGPATH="/nojournal/bin/path/config"
PROGNAME_WMEFWD="wmefwd"
GREPARGS_WMEFWD="obuWmefwd.config"
PROGARGS_WMEFWD="-c $CONFIGPATH/$GREPARGS_WMEFWD"
PROGNAME_OBUAWARE="obuAware"
GREPARGS_OBUAWARE="obuSocket.config"
PROGARGS_OBUAWARE="-s $CONFIGPATH/$GREPARGS_OBUAWARE -c $CONFIGPATH/obuVinBus1.config -f"
PROGNAME_BSMD="BSMd"

/usr/bin/killall -TERM  "$PROGNAME_BSMD"
sleep 1s
/usr/bin/killall  -TERM "$PROGNAME_WMEFWD"
sleep 1s
/usr/bin/killall  -TERM "$PROGNAME_OBUAWARE"
sleep 1s

while [ 1 ]; do
  if [ -e "$PROGPATH"/"$PROGNAME_WMEFWD" ];then
    ps -ef | grep "$PROGNAME_WMEFWD" | grep -v grep > /dev/null
		RETCODE=$?
		if [ $RETCODE -eq 1 ]; then
      echo "starting $PROGNAME_WMEFWD"
      exec "$PROGPATH/$PROGNAME_WMEFWD" $PROGARGS_WMEFWD &
			sleep 2s
		fi
  fi
  if [ -e "$PROGPATH"/"$PROGNAME_OBUAWARE" ];then
    ps -ef | grep "$PROGNAME_OBUAWARE" | grep -v grep > /dev/null
		RETCODE=$?
		if [ $RETCODE -eq 1 ]; then
      echo "starting $PROGNAME_OBUAWARE"
			exec "$PROGPATH/$PROGNAME_OBUAWARE" $PROGARGS_OBUAWARE &
			sleep 2s
		fi
  fi
  sleep 60s
done
