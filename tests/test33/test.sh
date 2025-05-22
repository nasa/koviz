#! /usr/bin/env bash

echo "------------------------------------------------"
echo "           Test sum/magnitude                   "
echo "------------------------------------------------"
echo ""
echo "Action1: Press 's' to sum"
echo "Expect1: Sum=[(0,10),(1,12),(2,14),(3,16),(4,18)"
echo "Action2: Close Plot"
echo "Action3: Choose DP_F again"
echo "Action4: Press 'm' for magnitude"
echo "Expect1: Sum=[(0,10),(1,~11.05),(2,~12.17),"
echo "              (3,~13.34),(4,~14.56)]"
koviz C.csv K.csv DP_F
