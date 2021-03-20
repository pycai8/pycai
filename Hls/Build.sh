#!/bin/bash

echo "===Start build Hls==="
g++ -g -shared -fPIC *.cpp -o libHls.so -I../Component -I../Network -L../Component -lComponent -lpthread
echo "===End build Hls==="

exit 0
