#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "          Test xy curve with log                         "
echo "---------------------------------------------------------"
echo ""
echo "Action1: Select curve and move to init point"
echo "Expect1: (100,100)"
echo ""
echo "Action2: Press right arrow repeatedly"
echo "Expect2: (100,100),(0,100),(-100,100),"
echo "         (-100,100),(0,100),(100,100)"
echo ""
echo "Action3: Press left arrow repeatedly"
echo "Expect3: See above points going back"
echo ""
echo "Action4: Roll over x-axis for log scale"
echo "Expect4: Single point (100,100)!"

koviz DP_test a.csv
