TESTDIR=/home/sean/network_tester/save
rm -rf $TESTDIR
mkdir $TESTDIR

# Messages
N=5

# IP Addresses
MACHINE1=192.168.1.101
MACHINE2=192.168.1.103
THISMACHINE=192.168.1.104

#Single Test
mkdir $TESTDIR/single
./tester.sh $MACHINE1 $MACHINE2 /dev/can1 /dev/can1 250 1 $N $TESTDIR/single/test $THISMACHINE
sleep 1

#Multiple Tests
for BAUD in 250 500
do
  mkdir $TESTDIR/baud$BAUD
  for RATE in 1 .5 .1 .05
  do
    mkdir $TESTDIR/baud$BAUD/rate$RATE
    ./tester.sh $MACHINE1 $MACHINE2 /dev/can1 /dev/can1 $BAUD $RATE $N $TESTDIR/baud$BAUD/rate$RATE/test $THISMACHINE
  sleep  1
  done
done
