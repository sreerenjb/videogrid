if [ "$EUID" -ne 0 ]
  then echo "Please run as a superuser (root)"
  exit
fi

ROOT_DIR=`pwd`
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ROOT_DIR/webserver/msdk_components/

echo "$LD_LIBRARY_PATH"
echo "############## Running VideoGridServer on port 8082 =>  "
$ROOT_DIR/webserver/VideoGridServer 8082

