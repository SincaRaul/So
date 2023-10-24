#!/bin/bash

if test "$#" -lt 3; then
  echo "Usage $0 with <numefisier> <dir> <file> n1 n2 n3..."
  exit 1
fi

fisier=$1
director=$2
shift 2
count=0

for arg; do
  if [[ "$arg" =~ ^[0-9]+$ ]]; then
    if ((arg > 9)); then
      ((count++))
    fi
  fi
done

echo "In total sunt $count numere mai mari decat 10"
sum=0
for arg; do
  if [[ "$arg" =~ ^[0-9]+$ ]]; then
    ((sum += arg))
  fi
done

cifre_suma=${#sum}  
echo "Suma numerelor este $sum"
echo "Suma are $cifre_suma cifre"

if test -f "$fisier"; then
  echo "$fisier e un fisier normal"
  echo "$count $sum" > "$fisier"
else
  echo "$fisier e un fisier nebun cu acte"
fi

for file in "$director"/*.txt; do
  cat "$file"
done

