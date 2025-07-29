#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "          Test ball curve with log xy                    "
echo "---------------------------------------------------------"
echo ""
echo "Action1: Select curve and move to init point"
echo "Expect1: (0.1,5.244)"
echo ""
echo "Action2: Select first local min"
echo "Expect2: (9.9,0.129)"
echo ""
echo "Action3: Select last point"
echo "Expect3: (22,1.181)"

koviz DP_test ball-posx.csv
