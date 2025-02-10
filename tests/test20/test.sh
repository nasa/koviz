#! /usr/bin/env bash

echo "-----------------------------------------------"
echo "Test when x and y come from different files"
echo "Also testing time match tolerance"
echo "Also testing different units in a.csv and b.csv"
echo "Also testing time names"
echo "-----------------------------------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect (10,0),(0,10),(-10,0),(0,-10),(10,0) - diamond "
/home/kvetter/dev/koviz/bin/koviz . \
              DP_test \
              -tmt 0.1 \
              -timeName "time=sys.exec.out.time=snow.time"
