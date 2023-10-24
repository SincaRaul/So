#!/bin/bash

while read linie; do
    if [ "$(echo $linie | grep -E "^[A-Z][a-zA-Z0-9\ \,]+\.{1}$" | grep -E -v "si[\ ]*\," | grep -E -v "n[pb]")" ]
    then echo "ok"
    else echo "not ok"
    fi
done