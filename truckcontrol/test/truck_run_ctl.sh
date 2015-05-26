#!/bin/sh
#
# Script to start truck control
# for BAA project 2008-10 

while getopts "V:" opt; do
case $opt in
    V) TRUCK=$OPTARG ;;
esac
done
if [[ $OPTIND -lt 3 ]] ; then
        echo 'No vehicle color specified (must be -V <Blue, Gold, or Silver)'
        exit
fi

if [[ $TRUCK != "Blue" && $TRUCK != "Gold" && $TRUCK != "Silvr" ]] ; then
        echo $TRUCK is neither Blue, Gold, nor Silvr
        exit
fi

echo optind $OPTIND

CONFIG_FILE='/home/truckcontrol/test/realtime.ini' ;
LONGITUDINAL='long_trk' ;
LONG_CFG_FILE='realtime' ;
COMMAND_TEST='1' ;

echo 'Run longitudinal control'
        slay  -f -Q -s TERM $LONGITUDINAL
        /home/truckcontrol/test/$LONGITUDINAL -v $TRUCK -f $CONFIG_FILE -o /big/data/$LONG_CFG_FILE -r $COMMAND_TEST >$LONG_CFG_FILE$COMMAND_TEST.log &

