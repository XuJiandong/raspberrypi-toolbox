#!/bin/bash

dir="$( cd "$( dirname "$0" )" && pwd )"
cd $dir

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$dir luajit main.lua $*

