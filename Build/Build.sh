#!/bin/bash

rm Info.log
(./InnerBuild.sh 2>>Info.log)>>Info.log
cat Info.log

exit 0;

