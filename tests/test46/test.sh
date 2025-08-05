#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "          Test time vs x and xy plot                     "
echo "---------------------------------------------------------"
echo ""
echo "Action1: Select curve and move to init point (use keys)"
echo "Expect1: Init point (50,0)"
echo ""
echo "Action2: Use right arrow to move through points"
echo "Expect2: (50,0),(50,0),(50,0),(50,100),"
echo "         (50,0),(50,0),(50,0),(50,-100),"
echo "         (50,0),(170,0),(290,100),(410,100),"
echo "         (530,100),(650,0),(770,-100),(890,-100),"
echo "         (1010,-100),(1130,0)"
echo "         "

echo "Action3: Use left arrow to move through points"
echo "Expect3: See points above"
echo ""
echo "Action4: Roll over x axis labels for xlog scale"
echo "Expect4: Same points as above"
echo ""
echo "Action5: Roll over y axis labels for xy/log scale"
echo "Expect5: (50,100),(50,100),(290,100),(410,100),"
echo "         (530,100),(770,100),(890,100),(1010,100)"

koviz DP_test_1 a.csv
