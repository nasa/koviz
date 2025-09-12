#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "  Test unicode, strings, emptyfields and dos ^M endings  "
echo ""
echo "  a.csv contains strings dog,cat. Koviz assigns int ids  "
echo "  to strings. First string encountered is \"dog\", so    "
echo "  dog=0, then cat=1 etc."
echo "---------------------------------------------------------"
echo ""
echo "Action1: Select curve"
echo "Expect1: Top curve (a):"
echo "         (0,945),(1,946),(2,0),(3,1),(5,947),(7,1)"
echo "         Bottom curve (b):"
echo "         (2,97),(3,98),(4,99)"

koviz DP_test a.csv
