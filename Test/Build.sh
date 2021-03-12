#!/bin/bash

echo "===start build test==="
g++ -g *.cpp -o test_rtsp -L../Component -lComponent -I../Component -I../Network
echo "===End build test==="

exit 0

