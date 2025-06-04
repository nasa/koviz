#! /usr/bin/env bash

echo "---------------------------------------------------"
echo "  Test Sum with interpolation and diff time units  "
echo "---------------------------------------------------"
echo " Note:  See points in *.ods"
echo ""
echo "Action1: Press 's' for sum"
echo "Expect1: sum=[(0,0),(1,3),(2,6),(3,15),(4,20),"
echo "              (5,25),(6,24),(7,28),(8,32),(9,27)]"
echo ""
koviz DP_test *.csv
