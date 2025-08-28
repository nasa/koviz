#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "          Test markers with culled log points            "
echo "---------------------------------------------------------"
echo ""
echo "Action1: Select curve and hover mouse over init point"
echo "Expect1: (0,1000)"
echo "          0=2*(2min*60sec/min)-240"
echo "          1000=2*(10m*100cm/m)-1000"
echo ""
echo "Action2: Press right arrow repeatedly to see all points"
echo "Expect2: (0,1000),(120,1000),(120,3000),(120,5000),"
echo "         (120,7000),(240,3000),(360,1000)"
echo ""
echo "Action3: Press left arrow repeatedly"
echo "Expect3: All points above in reverse"
echo ""
echo "Action4: Roll mouse over x axis to change to x-logscale"
echo "Expect4: (120,1000),(120,3000),(120,5000),"
echo "         (120,7000),(240,3000),(360,1000)"

koviz DP_test a.csv
