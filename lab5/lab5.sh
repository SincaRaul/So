#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Utilizare: $0 director output.txt"
    exit 1
fi

director="$1"
output_file="$2"

if [ ! -d "$director" ]; then
    echo "Directorul specificat nu exista."
    exit 1
fi

find "$director" -type f | while read -r arg; do
    if test -f "$arg" && [ "$(cat "$arg" | grep -E "^[A-Z][a-zA-Z0-9\ ,]+\.{1}$" | grep -E -v "si[\ ]*\," | grep -E -v "n[pb]" )" ]; then
        echo "$arg" >> "$output_file"
    fi
done

num_links=$(find "$director" -type l | wc -l)

echo "Numarul de legaturi simbolice: $num_links"

