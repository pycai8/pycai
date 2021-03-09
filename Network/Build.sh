#!/bin/bash

echo "===Start build Network==="
g++ -g -shared -fPIC *.cpp -o libNetwork.so -I. -I../Component -L../Component -lComponent
echo "===End build Network==="

exit 0
