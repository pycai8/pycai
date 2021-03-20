#!/bin/bash

echo "===start build test==="
g++ -g TestRtsp.cpp -o test_rtsp -L../Component -lComponent -I../Component -I../Network
g++ -g TestHls.cpp -o test_hls -L../Component -lComponent -I../Component -I../Network
echo "===End build test==="

exit 0

