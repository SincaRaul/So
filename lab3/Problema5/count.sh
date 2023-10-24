#!/bin/bash

director="$1"
output_file="$2"

total_characters=0

if [ ! -d "$director" ]; then
  echo "Directorul specificat nu există."
  exit 1
fi

for file in $(find "$director" -type f -name "*.txt"); do
  if [ "$file" != "$director/$output_file" ]; then
    numar_caractere=$(wc -m < "$file")
    echo "$file $numar_caractere" >> "$output_file"
    total_characters=$((total_characters + numar_caractere))
  fi
done

echo "TOTAL $total_characters" >> "$output_file"

