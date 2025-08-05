#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "          Test xy plot with multiple points at t=0       "
echo "                                                         "
echo "---------------------------------------------------------"
echo ""
echo "Action1: Select curve and move to init point (0,100)"
echo "Expect1: Flatline y=100 plot and init point (0,100)"
echo "         Y should be in logscale"
echo ""
echo "Action2: Use right arrow to move through points"
echo "Expect2: (0,100),(0,100),(100,100),(0,100)"
echo "         (-100,100),(-100,100),(0,100),(100,100)"   
echo ""
echo "Action3: Use left arrow to move through points"
echo "Expect3: See points above"
echo ""
echo "Action4: Roll over x/y axis labels for log/log scale"
echo "Expect4: Flatline (100,100)"

koviz DP_test a.csv
