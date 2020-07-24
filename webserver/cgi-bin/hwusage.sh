#!/bin/bash
echo "Content-type: text/html"
ROOT_DIR=`pwd`
cd $ROOT_DIR
echo
./webserver/cgi-bin/metrics_monitor
print "\n\t"
echo
