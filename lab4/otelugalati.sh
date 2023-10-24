#!/bin/bash

while read linie; do
    if [ "$(echo $linie | grep -E "([0-2][0-9]|[3][0-1]|10)/(0[1-9]|1[0-2])/([0-9]{4})\ ([0-1][0-9]|2[0-3])(:([0-5][0-9])){2}" )" ]; then
        echo "ok"
    else
        echo "not ok"
    fi
done

