#! /usr/bin/env bash

echo "-----------------------------------------"
echo "Test files with different frequencies    "
echo "-----------------------------------------"
echo "---------------------"
echo "Expect identical two value curves with following points:"
echo " (535.364,0),(535.3856,0),(535.8944,32768)"
echo " (536.9349999999999,32770),(542.8176,32770)"
echo " (542.8184,2),(542.9178000000001,2)"
echo "Try spacebar to see flatline"
echo "---------------------"
koviz DP_test RUN_1 RUN_2 -mapFile map.kvz -timeName "time=time1=time2"
