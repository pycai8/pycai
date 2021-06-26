#!/bin/bash

mkdir Bin
cd Bin

cp ../Test/test.h264 . -rf

cp ../3rd/ffmpeg/libavcodec.so.58  . -rf
cp ../3rd/ffmpeg/libavdevice.so.58  . -rf
cp ../3rd/ffmpeg/libavfilter.so.7  . -rf
cp ../3rd/ffmpeg/libavformat.so.58  . -rf
cp ../3rd/ffmpeg/libavutil.so.56  . -rf
cp ../3rd/ffmpeg/libswresample.so.3  . -rf
cp ../3rd/ffmpeg/libswscale.so.5  . -rf

cp ../3rd/ffmpeg/libavcodec.so.58  libavcodec.so -rf
cp ../3rd/ffmpeg/libavdevice.so.58  libavdevice.so -rf
cp ../3rd/ffmpeg/libavfilter.so.7  libavfilter.so -rf
cp ../3rd/ffmpeg/libavformat.so.58  libavformat.so -rf
cp ../3rd/ffmpeg/libavutil.so.56  libavutil.so -rf
cp ../3rd/ffmpeg/libswresample.so.3  libswresample.so -rf
cp ../3rd/ffmpeg/libswscale.so.5  libswscale.so -rf

cd ../Component
sh Build.sh

cd ../Network
sh Build.sh

cd ../Rtsp
sh Build.sh

cd ../Hls
sh Build.sh

cd ../HttpMjpg
sh Build.sh

cd ../Test
sh Build.sh

exit 0

