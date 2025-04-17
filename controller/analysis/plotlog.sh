#!/bin/sh

INTERVAL=600

while true; do
  echo "[$(date '+%Y-%m-%d %H:%M:%S')] Running PlotLog..."
  ./bin/PlotLog

  echo "[$(date '+%Y-%m-%d %H:%M:%S')] Waiting $INTERVAL seconds..."
  sleep $INTERVAL
done

