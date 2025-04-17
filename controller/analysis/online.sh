#!/bin/sh

# How to;  ./online.sh <input_file.root>

if [ $# -ne 1 ]; then
  echo "Usage: $0 <input_file.root>"
  exit 1
fi

INPUT_FILE="$1"
GOLD_FILE="rootfiles/run_20250416_235333.root"

if [ ! -f "$INPUT_FILE" ]; then
  echo "Error: Input file $INPUT_FILE not found."
  exit 1
fi

if [ ! -f "$GOLD_FILE" ]; then
  echo "Error: Input file $GOLD_FILE not found."
  exit 1
fi

FILENAME=`basename "$INPUT_FILE"`
BASENAME=`echo "$FILENAME" | sed 's/\.root$//'`

PDF_FILE="pdfs/${BASENAME}.pdf"
mkdir -p pdfs

echo "./bin/OnlineHist -f \"$INPUT_FILE\" -g \"$GOLD_FILE\" -p \"$PDF_FILE\""
./bin/OnlineHist -f "$INPUT_FILE" -g "$GOLD_FILE" -o "$PDF_FILE"
