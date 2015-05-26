#!/bin/sh
# Change CCVS messages (254, 241) so that a changing speed is inserted.
# Speed is not reasonable, just to test data recording.
awk 'BEGIN{x=0}{if (($3==254) && ($4==241)) {print $1,$2,$3,$4,$5,$6,x%100,x%100,$9, $10, $11, $12, $13; x++} else print $0}' eng.dat >doctored.dat
