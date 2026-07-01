#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "  Test Detach Tab                                        "
echo "---------------------------------------------------------"
echo ""
echo "Action1: View plot"
echo "Expect1: Two pages of plots - 3 on one and 1 on other"
echo "Action2: Select tab with label \"Detach Me\""
echo "Expect2: \"Detach Me\" Tab current page"
echo "Action3: Choose File > Detach Tab"
echo "Expect3: Tab detaches"
echo "Action4: Select a curve on each page and move mouse"
echo "Expect4: Plots synced in time"

koviz DP_detach a.csv
