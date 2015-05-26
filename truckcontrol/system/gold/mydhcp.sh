#!/bin/sh

# Script for starting dhcp.client. 
# 	-I 10 Try to connect 10 times
#	-u    Wait for connection to dhcp server before backgrounding dhcp.client
#	-m    Don't write to /etc/resolv.conf - keep config strings in memory 

/usr/sbin/dhcp.client -I 10 -u -m
