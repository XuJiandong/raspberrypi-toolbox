#!/bin/bash


dir="$( cd "$( dirname "$0" )" && pwd )"
cd $dir

WARNING_GPIO=25
function finish {
    ../rpi-toolbox util write $WARNING_GPIO 0 > /dev/null
    echo "turn off gpio port $WARNING_GPIO"
}
trap finish EXIT

function warning {
    echo "warning at `date`"
    for i in {1..3}; do
        ../rpi-toolbox util write $WARNING_GPIO 1 > /dev/null
        sleep 1
        ../rpi-toolbox util write $WARNING_GPIO 0 > /dev/null
        sleep 1
    done
}
while true; do
    if ../rpi-toolbox ud less 50; then
        warning
    else
        echo "error, rpi-toolbox failed"
    fi
done
