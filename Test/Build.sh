#!/bin/bash

echo "===start build test==="
g++ -g TestRtsp.cpp -o ../Bin/libRtsp.test -L../Bin -lComponent -I../Component -I../Network
g++ -g TestHls.cpp -o ../Bin/libHls.test -L../Bin -lComponent -I../Component -I../Network
echo "===End build test==="

exit 0

