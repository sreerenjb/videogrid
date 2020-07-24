#!/bin/bash
echo "Content-type: text/html"
ROOT_DIR=`pwd`
cd $ROOT_DIR/webserver/msdk_components
echo "ROOT===$ROOT_DIR"
echo
./metrics_monitor
echo
echo
