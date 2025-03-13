#! /usr/bin/env bash

echo "-----------------------------------------"
echo "Test files with different frequencies    "
echo "-----------------------------------------"
echo "---------------------"
echo "Expect value curve with following points:"
echo " (535.364,0),(535.3856,0),(535.8944,32768)"
echo " (536.9349999999999,32770),(542.8176,32770)"
echo " (542.8184,2),(542.9178000000001,2)"
echo "---------------------"
koviz DP_test RUN_test 
