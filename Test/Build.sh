#!/bin/bash

echo "===start build test==="
g++ -g TestRtsp.cpp -o ../Bin/test_rtsp -L../Bin -lComponent -I../Component -I../Network
g++ -g TestHls.cpp -o ../Bin/test_hls -L../Bin -lComponent -I../Component -I../Network
echo "===End build test==="

exit 0

