#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "          Test xy plot with F->C, scale/bias and log     "
echo "                                                         "
echo "  Values around 1e-9 off due to computer rounding errors "
echo "  when bias/scaling and 5/9 factor in F->C               "
echo "---------------------------------------------------------"
echo ""
echo "Action1: Select curve and move to init point (use keys)"
echo "Expect1: Window pane plot and init point (0,0)"
echo ""
echo "Action2: Use right arrow to move through points"
echo "Expect2: (0,0),(100,0),(0,0),(0,100),(0,0)"
echo "         (-100,0),(0,0),(0,-100),(0,0),"
echo "         (100,0),(100,100),(0,100),(-100,100),"
echo "         (-100,0),(-100,-100),(0,-100),"
echo "         (100,-100),(100,0)"

echo "Action3: Use left arrow to move through points"
echo "Expect3: See points above"
echo ""
echo "Action4: Roll over x/y axis tic labels for log/log scale"
echo "Expect4: Rectangle plot with (~0,~0) bottom left"
echo "         and (~100,~100) top right.  The reason points"
echo "         not culled is that F->C fuzzy about zero - "
echo "         but non-zero."

koviz DP_test a.csv
