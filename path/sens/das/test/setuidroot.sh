#!/bin/sh
EXECS="qnx/null_das qnx/rdiop qnx/wriop qnx/qmm10xt qnx/dmm32 qnx/garnet"
chown root $EXECS
chgrp root $EXECS
chmod 4775 $EXECS
