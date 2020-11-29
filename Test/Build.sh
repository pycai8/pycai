#!/bin/bash

echo "===start build test==="
g++ -g *.cpp -o Test -L../Component -lComponent -I../Component -I.
echo "===End build test==="

exit 0

