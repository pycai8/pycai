#!/bin/bash

echo "===Start build component===>"
cd ../Component
g++ -g -shared -fPIC Component.cpp CPycaiLogger.cpp CDllLoader.cpp SoMain.cpp -o ../Bin/libComponent.so -I../Component -I../Test
echo "===End build component===>"

echo "===start build test===>"
cd ../Test
g++ -g TestComponent.cpp -o ../Bin/TestComponent -L../Bin -lComponent -I../Component -I../Test
echo "===End build test===>"

cd ../Build

exit 0
