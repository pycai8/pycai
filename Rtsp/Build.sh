#!/bin/bash

echo "===Start build Rtsp==="
g++ -g -shared -fPIC *.cpp -o libRtsp.so -I../Component -I../Network -L../Component -I../Network -lComponent -lpthread
echo "===End build Rtsp==="

exit 0
