#!/bin/bash

mkdir Bin
cd Bin

cp ../Test/test.h264 . -rf

cd ../Component
sh Build.sh

cd ../Network
sh Build.sh

cd ../Rtsp
sh Build.sh

cd ../Hls
sh Build.sh

cd ../Test
sh Build.sh

exit 0

