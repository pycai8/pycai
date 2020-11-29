#!/bin/bash

echo "===Start build component==="
g++ -g -shared -fPIC *.cpp -o libComponent.so -I.
echo "===End build component==="

exit 0
