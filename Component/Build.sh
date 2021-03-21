#!/bin/bash

echo "===Start build component==="
g++ -g -shared -fPIC *.cpp -o ../Bin/libComponent.so -I. -ldl
echo "===End build component==="

exit 0
