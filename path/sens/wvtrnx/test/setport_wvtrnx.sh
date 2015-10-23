#!/bin/sh

# Configure Wavetronix serial port

/usr/bin/stty -parenb -parodd -hupcl -cstopb -crtscts -brkint -parmrk -inpck -istrip -inlcr -icrnl -ixon -ixoff -onlret -ofill -ofdel -isig -icanon -iexten -echoe -echok -echonl -noflsh -xcase -tostop -echoprt -echoctl -echoke >/dev/ttyS0 

/usr/bin/stty 9600 >/dev/ttyS0
/usr/bin/stty raw >/dev/ttyS0

    #Garmin GPS
#/usr/bin/stty -F /dev/ttyS4 -parenb -parodd cs8 -hupcl -cstopb cread clocal \
#           -crtscts ignbrk -brkint ignpar -parmrk -inpck -istrip -inlcr igncr \
#            -icrnl -ixon -ixoff -iuclc ixany -imaxbel -iutf8 -opost -olcuc \
#            -ocrnl -onlcr onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 \
#            -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase \
#            -tostop -echoprt -echoctl -echoke
#/usr/bin/stty -F /dev/ttyS4 19200

    #Summit 25203A accelerometer
#/usr/bin/stty -F /dev/ttyS5 9600 raw

    #Gyro FIX THIS WORKS BETTER stty raw </dev/ttyS6
#/usr/bin/stty -F /dev/ttyS6 -parenb -parodd cs8 -hupcl -cstopb cread clocal \
#            -crtscts ignbrk -brkint ignpar -parmrk -inpck -istrip -inlcr igncr \
#            -icrnl -ixon -ixoff -iuclc ixany -imaxbel -iutf8 -opost -olcuc \
#            -ocrnl -onlcr onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 \
#            -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase \
#            -tostop -echoprt -echoctl -echoke
#/usr/bin/stty -F /dev/ttyS6 raw
#/usr/bin/stty -F /dev/ttyS6 9600

    #Safetrac
#/usr/bin/stty -F /dev/ttyS7 -parenb -parodd cs8 -hupcl -cstopb cread clocal \
#            -crtscts ignbrk -brkint ignpar -parmrk -inpck -istrip -inlcr igncr \
#            -icrnl -ixon -ixoff -iuclc ixany -imaxbel -iutf8 -opost -olcuc \
#            -ocrnl -onlcr onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 \
#            -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase \
#            -tostop -echoprt -echoctl -echoke
#/usr/bin/stty -F /dev/ttyS7 19200

    #EVT300 radars
    #Right rear radar
#/usr/bin/stty -F /dev/ttyS8 -parenb -parodd cs8 -hupcl -cstopb cread clocal \
#            -crtscts ignbrk -brkint ignpar -parmrk -inpck -istrip -inlcr igncr \
#            -icrnl -ixon -ixoff -iuclc ixany -imaxbel -iutf8 -opost -olcuc \
#            -ocrnl -onlcr onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 \
#            -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase \
#            -tostop -echoprt -echoctl -echoke
#/usr/bin/stty -F /dev/ttyS8 19200

    #Right front radar
#/usr/bin/stty -F /dev/ttyS9 -parenb -parodd cs8 -hupcl -cstopb cread clocal \
#            -crtscts ignbrk -brkint ignpar -parmrk -inpck -istrip -inlcr igncr \
#            -icrnl -ixon -ixoff -iuclc ixany -imaxbel -iutf8 -opost -olcuc \
#            -ocrnl -onlcr onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 \
#            -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase \
#            -tostop -echoprt -echoctl -echoke
#/usr/bin/stty -F /dev/ttyS9 19200

    #Left rear radar
#/usr/bin/stty -F /dev/ttyS10 -parenb -parodd cs8 -hupcl -cstopb cread clocal \
#            -crtscts ignbrk -brkint ignpar -parmrk -inpck -istrip -inlcr igncr \
#            -icrnl -ixon -ixoff -iuclc ixany -imaxbel -iutf8 -opost -olcuc \
#            -ocrnl -onlcr onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 \
#            -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase \
#            -tostop -echoprt -echoctl -echoke
#/usr/bin/stty -F /dev/ttyS10 19200

    #Left front radar
#/usr/bin/stty -F /dev/ttyS11 -parenb -parodd cs8 -hupcl -cstopb cread clocal \
#            -crtscts ignbrk -brkint ignpar -parmrk -inpck -istrip -inlcr igncr \
#            -icrnl -ixon -ixoff -iuclc ixany -imaxbel -iutf8 -opost -olcuc \
#            -ocrnl -onlcr onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 \
#            -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase \
#            -tostop -echoprt -echoctl -echoke
#/usr/bin/stty -F /dev/ttyS11 19200

echo Done configuring Wavetronix serial port
