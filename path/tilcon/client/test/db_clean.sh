#!/bin/sh

#script to get rid of all Posix message queues
`ipcs -q | grep -v Message | grep -v key | awk '{printf("ipcrm -q %d\n", $2)}' | grep -v 'q 0'`

#get rid of files in the register
rm -f /home/path/db/register/dbq*

