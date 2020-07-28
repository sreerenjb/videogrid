#!/bin/bash
echo "Content-type: text/html"
ROOT_DIR=`pwd`
echo
LD_PRELOAD=./webserver/msdk_components/libcttmetrics.so ./webserver/msdk_components/metrics_monitor
echo
echo
