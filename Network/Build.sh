#!/bin/bash

echo "===Start build Network==="
g++ -g -shared -fPIC *.cpp -o ../Bin/libNetwork.so -I. -I../Component -I../Rtsp -L../Bin -lComponent -lpthread
echo "===End build Network==="

exit 0
