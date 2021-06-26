#!/bin/bash

echo "===Start build Hls==="
g++ -g -O0 -shared -fPIC *.cpp -o ../Bin/libHttpMjpg.so -I../Component -I../Network -I../3rd/ffmpeg -L../Bin -L../3rd/ffmpeg -lavcodec -lComponent -lpthread
echo "===End build Hls==="

exit 0
