#!/bin/bash


dir="$( cd "$( dirname "$0" )" && pwd )"
cd $dir

function finish {
    echo "done"
    ../rpi-toolbox util write 25 0
}
trap finish EXIT
while true; do
    ../rpi-toolbox util write 25 1
    sleep 1
    ../rpi-toolbox util write 25 0
    sleep 1
done

