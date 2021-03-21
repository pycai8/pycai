#!/bin/bash

echo "===Start build Hls==="
g++ -g -shared -fPIC *.cpp -o ../Bin/libHls.so -I../Component -I../Network -L../Bin -lComponent -lpthread
echo "===End build Hls==="

exit 0
