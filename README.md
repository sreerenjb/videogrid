# videogrid
Decode video streams in parallel, Render as grids &amp; Benchmark in browser

## Project details

The intention of the videogrid project is to easily benchmark the
H/W capability and usage of accelerated decoder pipeline via web interface.

Currently the benchmarking works only for the Intel hardware.

## How to run

### Linux

sudo ./run.sh

Open the "videogrid.html" on a browser, select the number of parallel streams
to be decoded & displayed, click the Play button.

To get the Hardware usage, click on Status button.

### ChromeOS

Download videogrid project to /home/chronos/user/Downloads

sudo mount /home/chronos/user -o remount,exec

sudo ./run.sh

Open the "videogrid.html" on a browser, select the number of parallel streams
to be decoded & displayed, click the Play button.

To get the Hardware usage, click on Status button.

## Design & Components

The videogrid project has 3 major components

1) HTML Web Interface for the UI (videogrid.html)

2) A modified version of tiny web server taken from http://www.cs.cmu.edu/afs/cs/academic/class/15213-s00/www/class28/tiny.c (webserver/videogrid_server.c)

3) Metrics montior lib & binary derived from Intel's MediaSDK project https://github.com/Intel-Media-SDK/MediaSDK/tree/master/samples/metrics_monitor (webserver/msdk_components/libcttmetrics.so  & webserver/msdk_components/metrics_monitor)
