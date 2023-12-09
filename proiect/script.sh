#!/bin/bash

if [ $# -ne 1 ]; then
    echo "EROARE: numar gresit de argumente"
    echo "USAGE: bash $0 <caracter>"
    exit 1
fi

caracter=$1
contor=0

while IFS= read -r linie || [[ -n "$linie" ]]; do
    aux=$(echo "$linie" | grep -E '^[A-Z][a-zA-Z0-9, -]*(\.|\?|\!)$' | grep -vE ',[ ]*si[ ]*' | grep -vE '[ ]*si[ ]*,')

    if [[ -n "$aux" && $linie == *"$caracter"* ]]; then
        ((contor++))
    fi

done

echo $contor
