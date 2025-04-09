#!/bin/sh

# How to;  ./offline.sh <input_file.dat|input_file.tar.xz>

if [ $# -ne 1 ]; then
  echo "Usage: $0 <input_file.dat|input_file.tar.xz>"
  exit 1
fi

INPUT_FILE="$1"

EXTENSION=`echo "$INPUT_FILE" | awk -F. '{print $NF}'`

if [ ! -f "$INPUT_FILE" ]; then
  echo "Error: Input file $INPUT_FILE not found."
  exit 1
fi

FILENAME=`basename "$INPUT_FILE"`
BASENAME=`echo "$FILENAME" | sed 's/\.tar\.xz$//' | sed 's/\.dat$//'`

ROOT_FILE="rootfiles/${BASENAME}.root"
PARAM_FILE="param/${BASENAME}.json"

if [ ! -f "$PARAM_FILE" ]; then
  echo "Warning: Parameter file $PARAM_FILE not found. Using param/default.json instead."
  PARAM_FILE="param/default.json"
fi

# 実行
echo "./bin/offline -f \"$INPUT_FILE\" -w \"$ROOT_FILE\" -p \"$PARAM_FILE\""
./bin/offline -f "$INPUT_FILE" -w "$ROOT_FILE" -p "$PARAM_FILE"
