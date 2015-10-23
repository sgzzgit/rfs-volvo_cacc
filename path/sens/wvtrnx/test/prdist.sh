#!/bin/sh
# Print out distances for all tracks on wavetronix log file
# operates on stdin

awk -f prdist.awk 
