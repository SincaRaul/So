#!/bin/bash

director="$1"
caracter="$2"

if [ ! -d "$director" ]; then
  echo "Directorul specificat nu exista."
  exit 1
fi

if [[ "$caracter" =~ [rwx] ]]; then
  find "$director" -type f -name "*.txt" -exec chmod u+"$caracter" {} \;
else
  echo "Caracterul specificat nu este valid: $caracter"
fi

