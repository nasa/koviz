#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "          Test markers with culled log points            "
echo "---------------------------------------------------------"
echo ""
echo "Action1: Select curve and use left arrow to go to start"
echo "Expect1: (0,1)"
echo ""
echo "Action2: Press right arrow repeatedly to see all points"
echo "Expect2: (0,1),(1,1],(1,2),(1,3),(1,4),"
echo "         (4,4.1),(4,5.1),(4,6.1),<4,7.1>,"
echo "         (4,7),(4,6),(4,5),(4,4),<5,1>,"
echo "         (9,2),(10,4)"
echo ""
echo "Action3: Press left arrow repeatedly"
echo "Expect3: All points above in reverse"

koviz DP_test a.csv
