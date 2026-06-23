#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "  Test Shared Windows                                    "
echo "---------------------------------------------------------"
echo ""
echo "Action1: Click a"
echo "Expect1: See step curve a"
echo "Action2: Choose File > New Window"
echo "Expect2: New empty window"
echo "Action3: Click b in new window"
echo "Expect3: See step curve b"
echo "Action4: Select a and b and move mouse"
echo "Expect4: Live coord synced over windows"
echo "Action5: On curve b use arrows"
echo "Expect5: Live coord synced over windows - especially over"
echo "         times that time stays constant on step"

koviz a.csv
