#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "          Test xy plot with duplicate times and log      "
echo "---------------------------------------------------------"
echo ""
echo "Action1: Select curve and move to init point (use keys)"
echo "Expect1: (100,50)"
echo ""
echo "Action2: Use right arrow to move through points"
echo "Expect2: (100,50),(100,100),(50,100),(0,100)"
echo ""
echo "Action3: Use left arrow to move through points"
echo "Expect3: See points above"
echo ""
echo "Action4: Roll over x axis label to make x axis logscale"
echo "Expect4: (100,50),(100,100),(50,100)"

koviz DP_test a.csv
