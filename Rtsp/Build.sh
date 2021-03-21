#!/bin/bash

echo "===Start build Rtsp==="
g++ -g -shared -fPIC *.cpp -o ../Bin/libRtsp.so -I../Component -I../Network -L../Bin -I../Network -lComponent -lpthread
echo "===End build Rtsp==="

exit 0
