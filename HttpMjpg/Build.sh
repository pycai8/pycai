#!/bin/bash

echo "===Start build HttpMjpg==="
g++ -g -O0 -shared -fPIC *.cpp -o ../Bin/libHttpMjpg.so -I../Component -I../Network -I../3rd/ffmpeg -L../Bin -L../3rd/ffmpeg -lavcodec -lComponent -lpthread
echo "===End build HttpMjpg==="

exit 0
