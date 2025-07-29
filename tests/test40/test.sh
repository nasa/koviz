#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "          Test error plot with culled log points         "
echo "---------------------------------------------------------"
echo ""
echo "Action1: Select curve and use left arrow to go to start"
echo "Expect1: First point is (0,-0.1)"
echo ""
echo "Action2: Press right arrow repeatedly to see all points"
echo "Expect2: (0,0.1),(1,0.1),(1,0.2),(1,0.3),(1,0.4)"
echo "         (4,0.3),(4,0.4),(4,0.5),(4,0.6),(4,0.7)"
echo "         (4,0.6),(4,0.5),(4,0.4),(5,0.1),(9,0.2)"
echo "         (10,0.4)"
echo ""
echo "Action3: Press left arrow repeatedly to see all points"
echo "Expect3: See all points above"
echo ""

koviz DP_test a.csv b.csv -pres error
