#!/bin/bash
echo "Content-type: text/html"
ROOT_DIR=`pwd`
cd $ROOT_DIR/webserver/cgi-bin/
echo
./metrics_monitor
print "\n\t"
echo
