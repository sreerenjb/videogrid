#!/bin/bash
if [ "$EUID" -ne 0 ];
  then echo "Please run as a superuser (root)"
  exit
fi

ROOT_DIR=`pwd`
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./webserver/msdk_components/

echo "############## Running VideoGridServer on port 8082 =>  "
./webserver/VideoGridServer 8082

