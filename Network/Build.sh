#!/bin/bash

echo "===Start build Network==="
g++ -g -shared -fPIC *.cpp -o libNetwork.so -I. -I../Component -I../Rtsp -L../Component -lComponent -lpthread
echo "===End build Network==="

exit 0
