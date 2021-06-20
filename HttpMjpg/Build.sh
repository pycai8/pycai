#!/bin/bash

echo "===Start build Hls==="
g++ -g -O0 -shared -fPIC *.cpp -o ../Bin/libHttpMjpg.so -I../Component -I../Network -L../Bin -lComponent -lpthread
echo "===End build Hls==="

exit 0
