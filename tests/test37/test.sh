#! /usr/bin/env bash

echo "---------------------------------------------------"
echo "              Test XY LiveTimeIdx                  "
echo "---------------------------------------------------"
echo ""
echo "Action1: N/A"
echo "Expect1: Diamond with spike off (0,10)->(0,12)"
echo ""
echo "Action2: Click (10,0) then use right arrow key to"
echo "         view points points on curve."
echo "Expect2: Points: (10,0),(0,10),(0,11),(0,12)"
echo "                 (0,10),(-10,0),(0,-10),(10,0)"
echo ""
koviz DP_xy a.csv
